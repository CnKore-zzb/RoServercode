#ifndef _XSTATEMACHINE
#define _XSTATEMACHINE

#include "xState.h"

template<typename entity_type>
class xStateMachine
{
	public:
		xStateMachine(entity_type *_owner, xState<entity_type> *_curState, 
				xState<entity_type> *_globalState = NULL) : owner(_owner) 
		{
			prevState = NULL;
			curState = _curState;
			globalState = _globalState;
		}
		virtual ~xStateMachine(){}

		void update() const
		{
			if (globalState) globalState->execute(owner);
			if (curState) curState->execute(owner);
		}

		void changeState(xState<entity_type> *newState)
		{
			if (!newState || curState == newState) return;
			prevState = curState;
			curState = newState;

			if (prevState)
				prevState->exit(owner);
			curState->enter(owner);
		}

		void changeGlobalState(xState<entity_type> *newState)
		{
			if (!newState) return;
			if (globalState)
				globalState->exit(owner);
			globalState = newState;
			globalState->enter(owner);
		}

		void revertToPreviousState()
		{
			changeState(prevState);
		}

		xState<entity_type> *getCurState()
		{
			return curState;
		}

	private:
		entity_type * const owner; //状态机持有者

		xState<entity_type> *prevState;		//前一状态
		xState<entity_type> *curState;		//当前状态
		xState<entity_type> *globalState;	//全局状态

};

#endif
