#include "SubMain/SubStateManager.h"

namespace subjugator
{
	StateManager::StateManager()
	{
		mCurrentState = SubStates::UNKNOWN;
		mPreviousState = SubStates::UNKNOWN;
		mCallbackList.resize(10);
	}

	void StateManager::SetStateCallback(int state, std::string name, boost::function<void(void)> callback)
	{
		if((size_t)state >= mCallbackList.size())
			mCallbackList.resize(state + 1);

		mCallbackList[state] = boost::shared_ptr<StateObject>(new StateObject(name, callback));
	}

	void StateManager::ChangeState(int newState)
	{
		assert(mCallbackList[newState]);

		mPreviousState = mCurrentState;
		mCurrentState = newState;
	}

	void StateManager::Execute()
	{
		// If we're in an unknown state, quietly exit
		if(mCurrentState == SubStates::UNKNOWN)
			return;

		// Call the current callback, if it exists
		if(mCallbackList[mCurrentState])
			mCallbackList[mCurrentState]->Execute();

		// Call the all callback, if it exists
		if(mCallbackList[SubStates::ALL])
			mCallbackList[SubStates::ALL]->Execute();
	}

	std::string StateManager::GetStateName(int state)
	{
		return mCallbackList[state]->Name;
	}
}
