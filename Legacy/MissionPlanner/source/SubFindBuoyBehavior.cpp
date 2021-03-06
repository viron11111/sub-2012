#include "MissionPlanner/SubFindBuoyBehavior.h"
#include "MissionPlanner/SubMissionPlannerWorker.h"
#include "MissionPlanner/AnnoyingConstants.h"

#include <iostream>

using namespace subjugator;
using namespace std;
using namespace Eigen;

FindBuoyBehavior::FindBuoyBehavior(double minDepth, bool goleft) :
	MissionBehavior(MissionBehaviors::FindBuoy, "FindBuoy", minDepth),
	canContinue(false),	bumpSet(false), backupSet(false), clearBuoysSet(false),
	pipeSet(false), hasSeenBuoy(0), yawChange(0), yawChangeSet(false), goLeft(goleft)
{
	gains2d = Vector2d(1.0, 1.0);

	// TODO enqueue which buoys we are looking for
	buoysToFind.push(BuoyAwesomeness(ObjectIDs::BuoyGreen, false));
	buoysToFind.push(BuoyAwesomeness(ObjectIDs::BuoyGreen, true));


	// Setup the callbacks
	stateManager.SetStateCallback(FindBuoyMiniBehaviors::ApproachBuoy,
			"ApproachBuoy",
			boost::bind(&FindBuoyBehavior::ApproachBuoy, this));
	stateManager.SetStateCallback(FindBuoyMiniBehaviors::BumpBuoy,
			"BumpBuoy",
			boost::bind(&FindBuoyBehavior::BumpBuoy, this));
	stateManager.SetStateCallback(FindBuoyMiniBehaviors::BackupMofoYouRanHerOver,
			"BackupMofoYouRanHerOver",
			boost::bind(&FindBuoyBehavior::BackupMofoYouRanHerOver, this));
	stateManager.SetStateCallback(FindBuoyMiniBehaviors::ClearBuoys,
			"ClearBuoys",
			boost::bind(&FindBuoyBehavior::ClearBuoys, this));
	stateManager.SetStateCallback(FindBuoyMiniBehaviors::DriveTowardsPipe,
			"DriveTowardsPipe",
			boost::bind(&FindBuoyBehavior::DriveTowardsPipe, this));
	stateManager.SetStateCallback(FindBuoyMiniBehaviors::PanForBuoy,
			"PanForBuoy",
			boost::bind(&FindBuoyBehavior::PanForBuoy, this));
}

void FindBuoyBehavior::Startup(MissionPlannerWorker& mpWorker)
{
	// Connect to the worker's 2d object signal
	connection2D = mpWorker.on2DCameraReceived.connect(boost::bind(&FindBuoyBehavior::Update2DCameraObjects, this, _1));
	// And become the controlling device of the camera
	mPlannerChangeCamObject = mpWorker.ConnectToCommand((int)MissionPlannerWorkerCommands::SendVisionID, 1);

	// Save our pipe heading
	pipeHeading = lposRPY(2);

	// Push to approach buoy
	stateManager.ChangeState(FindBuoyMiniBehaviors::ApproachBuoy);
}

void FindBuoyBehavior::Shutdown(MissionPlannerWorker& mpWorker)
{
	connection2D.disconnect();	// Play nicely and disconnect from the 2d camera signal

	if(boost::shared_ptr<InputToken> r = mPlannerChangeCamObject.lock())
	{
		// Tell the cameras to not look for anything
		VisionSetIDs todown(MissionCameraIDs::Down, std::vector<int>(1, ObjectIDs::None));
		VisionSetIDs tofront(MissionCameraIDs::Front, std::vector<int>(1, ObjectIDs::None));

		r->Operate(todown);
		r->Operate(tofront);

		// And disconnect from the camera command
		r->Disconnect();
	}
}

void FindBuoyBehavior::Update2DCameraObjects(const std::vector<FinderResult2D>& camObjects)
{
	lock.lock();

	objects2d = camObjects;

	newFrame = true;

	lock.unlock();
}

void FindBuoyBehavior::DoBehavior()
{
	// LPOS info is updated by the algorithm

	if(buoysToFind.size() > 0)
	{
		currentObjectID = buoysToFind.front().objectID;

		// Tell the down camera to not look for anything here
		VisionSetIDs todown(MissionCameraIDs::Down, std::vector<int>(1, ObjectIDs::None));
		// And let the front camera know of the current target
		VisionSetIDs tofront(MissionCameraIDs::Front, std::vector<int>(1, currentObjectID));

		if(boost::shared_ptr<InputToken> r = mPlannerChangeCamObject.lock())
		{
			r->Operate(todown);
			r->Operate(tofront);
		}
	}
	// The mini functions are called in the algorithm
}

void FindBuoyBehavior::ApproachBuoy()
{
	bool sawBuoy = false;
	if(!canContinue)
	{
		if(!newFrame)
			return;

		getGains();

		newFrame = false;

		// The list of 2d objects the class is holding is the current found images in the frame
		double bestU = 0;
		int bestIndex = 0;

		// The list of 2d objects the class is holding is the current found images in the frame
		for(size_t i = 0; i < objects2d.size(); i++)
		{
			if(objects2d[i].objectID == currentObjectID && objects2d[i].cameraID == MissionCameraIDs::Front)
			{
				// Populate bestU correctly the first time through.
				if (bestU == 0)
				{
					bestU = objects2d[i].u;
					bestIndex = i;
				}

				// Find correct buoy to travel to based on apriori information.
				if (buoysToFind.front().toRight) // Take largest u
				{
					if (objects2d[i].u > bestU)
					{
						bestU = objects2d[i].u;
						bestIndex = i;
					}
				}
				else
				{
					if (objects2d[i].u < bestU)
					{
						bestU = objects2d[i].u;
						bestIndex = i;
					}

				}

				sawBuoy = true;
			}
		}

		if(sawBuoy)
		{
			// The buoy we want is in view. Get the NED waypoint from the generator
			desiredWaypoint = wayGen->GenerateFrom2D(*lposInfo, lposRPY, objects2d[bestIndex], servoGains2d, 0.0, true);

			if(!desiredWaypoint)	// Bad find, waygen says no good
				return;

			double distance = 0.0;
			lastScale = objects2d[bestIndex].scale;
			if(objects2d[bestIndex].scale >= approachThreshold)
				canContinue = true;
			else
				distance = approachTravelDistance;

			// Project the distance in the X,Y plane
			Vector3d distanceToTravel(distance*cos(desiredWaypoint->RPY(2)),
					distance*sin(desiredWaypoint->RPY(2)),
					0.0);	// Use the servo'd z depth

			desiredWaypoint->Position_NED += distanceToTravel;
			desiredWaypoint->number = getNextWaypointNum();

			hasSeenBuoy = 0;
		}
		// We either never saw the buoy or we lost it. Keep searching forward at pipe heading
		else
		{
			//if((hasSeenBuoy++) > 10)
			//	stateManager.ChangeState(FindBuoyMiniBehaviors::PanForBuoy);
			//else
			{
				double serioslycpp = approachTravelDistance;
				desiredWaypoint = boost::shared_ptr<Waypoint>(new Waypoint());
				desiredWaypoint->isRelative = false;
				desiredWaypoint->Position_NED = MILQuaternionOps::QuatRotate(lposInfo->getQuat_NED_B(),
						Vector3d(serioslycpp, 0.0, 0.0)) + lposInfo->getPosition_NED();
				desiredWaypoint->Position_NED(2) = approachDepth;
				desiredWaypoint->RPY = Vector3d(0.0, 0.0, pipeHeading);
				desiredWaypoint->number = getNextWaypointNum();
			}
		}
	}
	// Just waiting to arrive at the final waypoint for the mini behavior
	else
	{
		// Check to see if we have arrived
		if(atDesiredWaypoint())
		{
			// Done approaching the current buoy, switch to bump
			stateManager.ChangeState(FindBuoyMiniBehaviors::BumpBuoy);
		}
	}
}

void FindBuoyBehavior::BumpBuoy()
{
	if(!bumpSet)
	{
		double serioslycpp = bumpTravelDistance;
		desiredWaypoint = boost::shared_ptr<Waypoint>(new Waypoint());
		desiredWaypoint->isRelative = false;
		desiredWaypoint->RPY(2) = lposRPY(2);

		// Add on the bump travel
		desiredWaypoint->Position_NED = lposInfo->getPosition_NED()
				+ MILQuaternionOps::QuatRotate(lposInfo->getQuat_NED_B(),
									Vector3d(serioslycpp, 0.0, 0.0));
		desiredWaypoint->number = getNextWaypointNum();

		bumpSet = true;
	}

	// Check to see if we have arrived at the bump point
	if(atDesiredWaypoint())
	{
		clearBuoysSet = false;
		bumpSet = false;
		backupSet = false;

		// Done bumping the current buoy, remove it from the list
		cout <<"Size before pop: " << buoysToFind.size() << endl;
		buoysToFind.pop();

		if(buoysToFind.size() > 0)
			stateManager.ChangeState(FindBuoyMiniBehaviors::BackupMofoYouRanHerOver);
		else
			stateManager.ChangeState(FindBuoyMiniBehaviors::ClearBuoys);
	}
}

void FindBuoyBehavior::BackupMofoYouRanHerOver()
{
	if(!backupSet)
	{
		desiredWaypoint = boost::shared_ptr<Waypoint>(new Waypoint());
		desiredWaypoint->isRelative = false;
		desiredWaypoint->RPY(2) = pipeHeading;

		// Add on the retract travel
		desiredWaypoint->Position_NED = lposInfo->getPosition_NED()
				- Vector3d(backupTravelDistance*cos(pipeHeading),
						   backupTravelDistance*sin(pipeHeading),
						   0.0);
		desiredWaypoint->number = getNextWaypointNum();

		backupSet = true;
	}

	// Check to see if we have arrived at the backup point
	if(atDesiredWaypoint(0.3))
	{
		backupSet = false;
		canContinue = false;
		yawChange = 0.0;
		alignDepth = 0.0;

		stateManager.ChangeState(FindBuoyMiniBehaviors::ApproachBuoy);
	}
}

void FindBuoyBehavior::ClearBuoys()
{
	if(!clearBuoysSet)
	{
		desiredWaypoint = boost::shared_ptr<Waypoint>(new Waypoint());
		desiredWaypoint->isRelative = false;
		desiredWaypoint->RPY(2) = pipeHeading;

		// Add on the retract travel
		desiredWaypoint->Position_NED = lposInfo->getPosition_NED();
		desiredWaypoint->Position_NED(2) = clearBuoysDepth;
		desiredWaypoint->number = getNextWaypointNum();

		clearBuoysSet = true;
	}

	// Check to see if we have arrived at the clear point
	if(atDesiredWaypoint())
	{
		clearBuoysSet = false;

		stateManager.ChangeState(FindBuoyMiniBehaviors::DriveTowardsPipe);
	}
}

void FindBuoyBehavior::DriveTowardsPipe()
{
	if(!pipeSet)
	{
		double seriouslycppX = driveTowardsPipeDistanceX;
		double seriouslycppY = driveTowardsPipeDistanceY;
		desiredWaypoint = boost::shared_ptr<Waypoint>(new Waypoint());
		desiredWaypoint->isRelative = false;
		desiredWaypoint->RPY(2) = pipeHeading;

		// Add on the bump travel
		desiredWaypoint->Position_NED = lposInfo->getPosition_NED()
				+ MILQuaternionOps::QuatRotate(lposInfo->getQuat_NED_B(),
									Vector3d(seriouslycppX, seriouslycppY, 0.0));
		desiredWaypoint->Position_NED(2) = clearBuoysDepth;
		desiredWaypoint->number = getNextWaypointNum();

		pipeSet = true;
	}

	// Check to see if we have arrived at the clear point
	if(atDesiredWaypoint())
	{
		pipeSet = false;

		// We've found all the buoys! - the behavior shutdown will be called when the worker pops us off the list
		behDone = true;
	}
}

void FindBuoyBehavior::PanForBuoy()
{ // put in the counter here, maybe just a constructor arg that sets left or right
	bool sawBuoy = false;
	hasSeenBuoy = 0;

	// The list of 2d objects the class is holding is the current found images in the frame
	for(size_t i = 0; i < objects2d.size(); i++)
	{
		if(objects2d[i].objectID == currentObjectID)
		{
			sawBuoy = true;
			if(hasSeenBuoy++ > 10)
			{
				hasSeenBuoy = 0;
				stateManager.ChangeState(FindBuoyMiniBehaviors::ApproachBuoy);
			}
			break;
		}
	}

	// We either never saw the buoy or we lost it.
	if(!sawBuoy)
	{
		lock.lock();

		if(alignDepth == 0.0)
			alignDepth = lposInfo->position_NED(2);

		desiredWaypoint = boost::shared_ptr<Waypoint>(new Waypoint());
		desiredWaypoint->isRelative = false;
		desiredWaypoint->Position_NED = lposInfo->getPosition_NED();
		desiredWaypoint->Position_NED(2) = alignDepth;

		if(!goLeft)
		{
			desiredWaypoint->RPY(2) = AttitudeHelpers::DAngleClamp(lposRPY(2) + yawSearchAngle*boost::math::constants::pi<double>());
		}
		else
		{
			desiredWaypoint->RPY(2) = AttitudeHelpers::DAngleClamp(lposRPY(2) - yawSearchAngle*boost::math::constants::pi<double>());
		}
		desiredWaypoint->number = getNextWaypointNum();

		lock.unlock();
	}

	// TODO what if we can't pan and find the buoy?
}

// TODO Set travel distances
void FindBuoyBehavior::getGains()
{

	if (lastScale > 5000)
	{
		servoGains2d = Vector2d(0.02*boost::math::constants::pi<double>() / 180.0, 0.003);
		approachTravelDistance = 0.2; // m
	}
	else
	{
		servoGains2d = Vector2d( .05*boost::math::constants::pi<double>() / 180.0, 0.0035);
		approachTravelDistance = .8; // m
	}
}
