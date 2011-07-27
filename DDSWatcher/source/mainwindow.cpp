#include "DDSWatcher/mainwindow.h"
#include "ui_mainwindow.h"

using namespace subjugator;
using namespace std;
using namespace Eigen;
using namespace boost;
using namespace boost::posix_time;

MainWindow::MainWindow(DDSDomainParticipant *participant, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    lposvssreceiver(participant, "LPOSVSS", bind(&MainWindow::LPOSVSSDDSReadCallback, this, _1)),
    setwaypointreceiver(participant, "SetWaypoint", bind(&MainWindow::SetWaypointDDSReadCallback, this, _1)),
    depthreceiver(participant, "Depth", bind(&MainWindow::DepthDDSReadCallback, this, _1)),
    imureceiver(participant, "IMU", bind(&MainWindow::IMUDDSReadCallback, this, _1)),
    dvlreceiver(participant, "DVL", bind(&MainWindow::DVLDDSReadCallback, this, _1)),
    pdstatusreceiver(participant, "PDStatus", bind(&MainWindow::PDStatusDDSReadCallback, this, _1)),
    hydrophonereceiver(participant, "Hydrophone", bind(&MainWindow::HydrophoneDDSReadCallback, this, _1)),
    findermessagelistreceiver(participant, "Vision", bind(&MainWindow::FinderMessageListDDSReadCallback, this, _1)),
    trajectoryreceiver(participant, "Trajectory", bind(&MainWindow::TrajectoryDDSReadCallback, this, _1)),
    lposvssData(false),
	setwaypointData(false),
	depthData(false),
	imuData(false),
	dvlData(false),
	pdstatusData(false),
	hydrophoneData(false),
	findermessagelistData(false),
	trajectoryData(false),
	logging(false)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("DDSWatcher"));

    // CONNECT DDS SIGNALS
    connect(this, SIGNAL(lposvssInfoReceived()), this, SLOT(onLPOSVSSInfoReceived()));
    connect(this, SIGNAL(setWaypointInfoReceived()), this, SLOT(onSetWaypointInfoReceived()));
    connect(this, SIGNAL(depthInfoReceived()), this, SLOT(onDepthInfoReceived()));
    connect(this, SIGNAL(imuInfoReceived()), this, SLOT(onIMUInfoReceived()));
    connect(this, SIGNAL(dvlInfoReceived()), this, SLOT(onDVLInfoReceived()));
    connect(this, SIGNAL(pdstatusInfoReceived()), this, SLOT(onPDStatusInfoReceived()));
    connect(this, SIGNAL(hydrophoneInfoReceived()), this, SLOT(onHydrophoneInfoReceived()));
    connect(this, SIGNAL(findermessagelistInfoReceived()), this, SLOT(onFinderMessageListInfoReceived()));
    connect(this, SIGNAL(trajectoryInfoReceived()), this, SLOT(onTrajectoryInfoReceived()));
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnBrowse_clicked()
{
	ui->logFileEdit->setText(QFileDialog::getSaveFileName(this));
}

void MainWindow::on_btnStartLog_clicked()
{
	logstream.open(ui->logFileEdit->text().toUtf8().constData());
	logstream << "LPOSPOSX, LPOSPOSY, LPOSPOSZ, LPOSQUAT0, LPOSQUAT1, LPOSQUAT2, LPOSQUAT3, LPOSVELX, LPOSVELY, LPOSVELZ, ";
	logstream << "LPOSANGRATE0, LPOSANGRATE1, LPOSANGRATE2, LPOSACCEL0, LPOSACCEL1, LPOSACCEL2, TRAJXD0, TRAJXD1, TRAJXD2, ";
	logstream << "TRAJXD3, TRAJXD4, TRAJXD5, TRAJXD_DOT0, TRAJXD_DOT1, TRAJXD_DOT2, TRAJXD_DOT3, TRAJXD_DOT4, TRAJXD_DOT5" << endl;
	ui->lblLogging->setText("Yes");

	logging = true;
}

void MainWindow::on_btnStopLog_clicked()
{
	logstream.close();
	ui->lblLogging->setText("No");

	logging = false;
}

void MainWindow::logData()
{
	if (logging)
	{
		logstream << second_clock::local_time().time_of_day() << ", ";
		logstream << ui->lblLPOSPosition0->text().toStdString() << ", ";
		logstream << ui->lblLPOSPosition1->text().toStdString() << ", ";
		logstream << ui->lblLPOSPosition2->text().toStdString() << ", ";
		logstream << ui->lblLPOSQuaternion0->text().toStdString() << ", ";
		logstream << ui->lblLPOSQuaternion1->text().toStdString() << ", ";
		logstream << ui->lblLPOSQuaternion2->text().toStdString() << ", ";
		logstream << ui->lblLPOSQuaternion3->text().toStdString() << ", ";
		logstream << ui->lblLPOSVelocity0->text().toStdString() << ", ";
		logstream << ui->lblLPOSVelocity1->text().toStdString() << ", ";
		logstream << ui->lblLPOSVelocity2->text().toStdString() << ", ";
		logstream << ui->lblLPOSAngular0->text().toStdString() << ", ";
		logstream << ui->lblLPOSAngular1->text().toStdString() << ", ";
		logstream << ui->lblLPOSAngular2->text().toStdString() << ", ";
		logstream << ui->lblLPOSAcceleration0->text().toStdString() << ", ";
		logstream << ui->lblLPOSAcceleration1->text().toStdString() << ", ";
		logstream << ui->lblLPOSAcceleration2->text().toStdString() << ", ";
		logstream << ui->lblTrajectoryXd0->text().toStdString() << ", ";
		logstream << ui->lblTrajectoryXd1->text().toStdString() << ", ";
		logstream << ui->lblTrajectoryXd2->text().toStdString() << ", ";
		logstream << ui->lblTrajectoryXd3->text().toStdString() << ", ";
		logstream << ui->lblTrajectoryXd4->text().toStdString() << ", ";
		logstream << ui->lblTrajectoryXd5->text().toStdString() << ", ";
		logstream << ui->lblTrajectoryXddot0->text().toStdString() << ", ";
		logstream << ui->lblTrajectoryXddot1->text().toStdString() << ", ";
		logstream << ui->lblTrajectoryXddot2->text().toStdString() << ", ";
		logstream << ui->lblTrajectoryXddot3->text().toStdString() << ", ";
		logstream << ui->lblTrajectoryXddot4->text().toStdString() << ", ";
		logstream << ui->lblTrajectoryXddot5->text().toStdString() << ", " << endl;
	}
}



//***************************************************************************
// On Received Functions
//***************************************************************************
void MainWindow::onLPOSVSSInfoReceived()
{
    ui->lblLPOSTimestamp->setText(QString::number(lposvssmsg.timestamp));
    ui->lblLPOSState->setText(QString::number(lposvssmsg.state));
    ui->lblLPOSPosition0->setText(QString::number(lposvssmsg.position_NED[0]));
    ui->lblLPOSPosition1->setText(QString::number(lposvssmsg.position_NED[1]));
    ui->lblLPOSPosition2->setText(QString::number(lposvssmsg.position_NED[2]));
    ui->lblLPOSQuaternion0->setText(QString::number(lposvssmsg.quaternion_NED_B[0]));
    ui->lblLPOSQuaternion1->setText(QString::number(lposvssmsg.quaternion_NED_B[1]));
    ui->lblLPOSQuaternion2->setText(QString::number(lposvssmsg.quaternion_NED_B[2]));
    ui->lblLPOSQuaternion3->setText(QString::number(lposvssmsg.quaternion_NED_B[3]));
    ui->lblLPOSVelocity0->setText(QString::number(lposvssmsg.velocity_NED[0]));
    ui->lblLPOSVelocity1->setText(QString::number(lposvssmsg.velocity_NED[1]));
    ui->lblLPOSVelocity2->setText(QString::number(lposvssmsg.velocity_NED[2]));
    ui->lblLPOSAngular0->setText(QString::number(lposvssmsg.angularRate_BODY[0]));
    ui->lblLPOSAngular1->setText(QString::number(lposvssmsg.angularRate_BODY[1]));
    ui->lblLPOSAngular2->setText(QString::number(lposvssmsg.angularRate_BODY[2]));
    ui->lblLPOSAcceleration0->setText(QString::number(lposvssmsg.acceleration_BODY[0]));
    ui->lblLPOSAcceleration1->setText(QString::number(lposvssmsg.acceleration_BODY[1]));
    ui->lblLPOSAcceleration2->setText(QString::number(lposvssmsg.acceleration_BODY[2]));

    logData();
}

void MainWindow::onSetWaypointInfoReceived()
{
	ui->lblSetWayptisRelative->setText(QString::number(setwaypointmsg.isRelative));
	ui->lblSetWayptPosition0->setText(QString::number(setwaypointmsg.position_ned[0]));
	ui->lblSetWayptPosition1->setText(QString::number(setwaypointmsg.position_ned[1]));
	ui->lblSetWayptPosition2->setText(QString::number(setwaypointmsg.position_ned[2]));
	ui->lblSetWayptRPY0->setText(QString::number(setwaypointmsg.rpy[0]));
	ui->lblSetWayptRPY1->setText(QString::number(setwaypointmsg.rpy[1]));
	ui->lblSetWayptRPY2->setText(QString::number(setwaypointmsg.rpy[2]));
}

void MainWindow::onDepthInfoReceived()
{
	ui->lblDepthTimestamp->setText(QString::number(depthmsg.timestamp));
	ui->lblDepthDepth->setText(QString::number(depthmsg.depth));
	ui->lblDepthHumidity->setText(QString::number(depthmsg.humidity));
	ui->lblDepthThermisterTemp->setText(QString::number(depthmsg.thermistertemp));
	ui->lbDepthHumidTemp->setText(QString::number(depthmsg.humiditytemp));
}

void MainWindow::onIMUInfoReceived()
{
	ui->lblIMUTimestamp->setText(QString::number(imumsg.timestamp));
	ui->lblIMUFlags->setText(QString::number(imumsg.flags));
	ui->lblIMUtemp->setText(QString::number(imumsg.temp));
	ui->lblIMUSupply->setText(QString::number(imumsg.supply));
	ui->lblIMUAcceleration0->setText(QString::number(imumsg.acceleration[0]));
	ui->lblIMUAcceleration1->setText(QString::number(imumsg.acceleration[1]));
	ui->lblIMUAcceleration2->setText(QString::number(imumsg.acceleration[2]));
	ui->lblIMUAngular0->setText(QString::number(imumsg.angular_rate[0]));
	ui->lblIMUAngular1->setText(QString::number(imumsg.angular_rate[1]));
	ui->lblIMUAngular2->setText(QString::number(imumsg.angular_rate[2]));
	ui->lblIMUMagField0->setText(QString::number(imumsg.mag_field[0]));
	ui->lblIMUMagField1->setText(QString::number(imumsg.mag_field[1]));
	ui->lblIMUMagField2->setText(QString::number(imumsg.mag_field[2]));
}

void MainWindow::onDVLInfoReceived()
{
	ui->lblDVLTimestamp->setText(QString::number(dvlmsg.timestamp));
	ui->lblDVLVelocity0->setText(QString::number(dvlmsg.velocity[0]));
	ui->lblDVLVelocity1->setText(QString::number(dvlmsg.velocity[1]));
	ui->lblDVLVelocity2->setText(QString::number(dvlmsg.velocity[2]));
	ui->lblDVLVelocityError->setText(QString::number(dvlmsg.velocityerror));
	ui->lblDVLHeight->setText(QString::number(dvlmsg.height));
	ui->lblDVLBeamCorrelation0->setText(QString::number(dvlmsg.beamcorrelation[0]));
	ui->lblDVLBeamCorrelation1->setText(QString::number(dvlmsg.beamcorrelation[1]));
	ui->lblDVLBeamCorrelation2->setText(QString::number(dvlmsg.beamcorrelation[2]));
	ui->lblDVLBeamCorrelation3->setText(QString::number(dvlmsg.beamcorrelation[3]));

	if (dvlmsg.good)
		ui->lblDVLGood->setText("True");
	else
		ui->lblDVLGood->setText("False");
}

void MainWindow::onPDStatusInfoReceived()
{
	ui->lblPDStatusTimestamp->setText(QString::number(pdstatusmsg.timestamp));
	ui->lblPDStatusState->setText(QString::number(pdstatusmsg.state));

	if (pdstatusmsg.estop)
		ui->lblPDStatusEstop->setText("True");
	else
		ui->lblPDStatusEstop->setText("False");

	ui->lblPDStatusCurrent0->setText(QString::number(pdstatusmsg.current[0]));
	ui->lblPDStatusCurrent1->setText(QString::number(pdstatusmsg.current[1]));
	ui->lblPDStatusCurrent2->setText(QString::number(pdstatusmsg.current[2]));
	ui->lblPDStatusCurrent3->setText(QString::number(pdstatusmsg.current[3]));
	ui->lblPDStatusCurrent4->setText(QString::number(pdstatusmsg.current[4]));
	ui->lblPDStatusCurrent5->setText(QString::number(pdstatusmsg.current[5]));
	ui->lblPDStatusCurrent6->setText(QString::number(pdstatusmsg.current[6]));
	ui->lblPDStatusCurrent7->setText(QString::number(pdstatusmsg.current[7]));
	ui->lblPDStatusTickcount->setText(QString::number(pdstatusmsg.tickcount));
	ui->lblPDStatusFlags->setText(QString::number(pdstatusmsg.flags));
	ui->lblPDStatusCurrent16->setText(QString::number(pdstatusmsg.current16));
	ui->lblPDStatusVoltage16->setText(QString::number(pdstatusmsg.voltage16));
	ui->lblPDStatusCurrent32->setText(QString::number(pdstatusmsg.current32));
	ui->lblPDStatusVoltage32->setText(QString::number(pdstatusmsg.voltage32));
}

void MainWindow::onHydrophoneInfoReceived()
{
	ui->lblHydrophoneTimestamp->setText(QString::number(hydrophonemsg.timestamp));
	ui->lblHydrophoneDeclination->setText(QString::number(hydrophonemsg.declination));
	ui->lblHydrophoneHeading->setText(QString::number(hydrophonemsg.heading));
	ui->lblHydrophoneDistance->setText(QString::number(hydrophonemsg.distance));
	ui->lblHydrophoneFrequency->setText(QString::number(hydrophonemsg.frequency));

	if (hydrophonemsg.valid)
		ui->lblHydrophoneValid->setText("True");
	else
		ui->lblHydrophoneValid->setText("False");
}

void MainWindow::onFinderMessageListInfoReceived()
{
	int numOf2dMsgs = findermessagelistmsg.messages2d.length();
	int numOf3dMsgs = findermessagelistmsg.messages3d.length();

	ui->lblVision2DNumOfObects->setText(QString::number(numOf2dMsgs));
	ui->lblVision3DNumOfObects->setText(QString::number(numOf3dMsgs));

	if (numOf2dMsgs > 0)
	{
		int value2D = ui->spinVision2D->value();

		if (value2D > (numOf2dMsgs - 1))
		{
			value2D = 0;
			ui->spinVision2D->setValue(0);
		}

		Finder2DMessage &msg2D = findermessagelistmsg.messages2d[value2D];

		ui->lblVision2DObjectID->setText(QString::number(msg2D.objectid));
		ui->lblVision2Du->setText(QString::number(msg2D.u));
		ui->lblVision2Dv->setText(QString::number(msg2D.v));
		ui->lblVision2DScale->setText(QString::number(msg2D.scale));
		ui->lblVision2DAngle->setText(QString::number(msg2D.angle));
	}

	if (numOf3dMsgs > 0)
	{
		int value3D = ui->spinVision3D->value();

		if (value3D > (numOf3dMsgs - 1))
		{
			value3D = 0;
			ui->spinVision3D->setValue(0);
		}

		Finder3DMessage &msg3D = findermessagelistmsg.messages3d[value3D];

		ui->lblVision3DObjectID->setText(QString::number(msg3D.objectid));
		ui->lblVision3Dx->setText(QString::number(msg3D.x));
		ui->lblVision3Dy->setText(QString::number(msg3D.y));
		ui->lblVision3Dz->setText(QString::number(msg3D.z));
		ui->lblVision3Dang1->setText(QString::number(msg3D.ang1));
		ui->lblVision3Dang2->setText(QString::number(msg3D.ang2));
		ui->lblVision3Dang3->setText(QString::number(msg3D.ang3));
	}
}


void MainWindow::onTrajectoryInfoReceived()
{
	ui->lblTrajectoryXd0->setText(QString::number(trajectorymsg.xd[0]));
	ui->lblTrajectoryXd1->setText(QString::number(trajectorymsg.xd[1]));
	ui->lblTrajectoryXd2->setText(QString::number(trajectorymsg.xd[2]));
	ui->lblTrajectoryXd3->setText(QString::number(trajectorymsg.xd[3]));
	ui->lblTrajectoryXd4->setText(QString::number(trajectorymsg.xd[4]));
	ui->lblTrajectoryXd5->setText(QString::number(trajectorymsg.xd[5]));

	ui->lblTrajectoryXddot0->setText(QString::number(trajectorymsg.xd_dot[0]));
	ui->lblTrajectoryXddot1->setText(QString::number(trajectorymsg.xd_dot[1]));
	ui->lblTrajectoryXddot2->setText(QString::number(trajectorymsg.xd_dot[2]));
	ui->lblTrajectoryXddot3->setText(QString::number(trajectorymsg.xd_dot[3]));
	ui->lblTrajectoryXddot4->setText(QString::number(trajectorymsg.xd_dot[4]));
	ui->lblTrajectoryXddot5->setText(QString::number(trajectorymsg.xd_dot[5]));

	logData();
}

//***************************************************************************
// CALLBACKS
//***************************************************************************
void MainWindow::LPOSVSSDDSReadCallback(const LPOSVSSMessage &msg)
{
	lposvssmsg = msg;
	emit lposvssInfoReceived();
}

void MainWindow::SetWaypointDDSReadCallback(const SetWaypointMessage &msg)
{
	setwaypointmsg = msg;
	emit setWaypointInfoReceived();
}

void MainWindow::DepthDDSReadCallback(const DepthMessage &msg)
{
	depthmsg = msg;
	emit depthInfoReceived();
}

void MainWindow::IMUDDSReadCallback(const IMUMessage &msg)
{
	imumsg = msg;
	emit imuInfoReceived();
}

void MainWindow::DVLDDSReadCallback(const DVLMessage &msg)
{
	dvlmsg = msg;
	emit dvlInfoReceived();
}

void MainWindow::PDStatusDDSReadCallback(const PDStatusMessage &msg)
{
	pdstatusmsg = msg;
	emit pdstatusInfoReceived();
}

void MainWindow::HydrophoneDDSReadCallback(const HydrophoneMessage &msg)
{
	hydrophonemsg = msg;
	emit hydrophoneInfoReceived();
}

void MainWindow::FinderMessageListDDSReadCallback(const FinderMessageList &msg)
{
	findermessagelistmsg = msg;
	emit findermessagelistInfoReceived();
}

void MainWindow::TrajectoryDDSReadCallback(const TrajectoryMessage &msg)
{
	trajectorymsg = msg;
	emit trajectoryInfoReceived();
}
