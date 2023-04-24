#ifndef _XGOAL_H
#define _XGOAL_H

#include "xDefine.h"

enum GoalStateEnum 
{
	Goal_inactive,
	Goal_active,
	Goal_completed,
	Goal_failed,
};

template <class entity_type>
class xGoal
{
	public:

		xGoal(entity_type *pE, WORD _type) : type(_type), owner(pE), status(Goal_inactive)
		{}

		virtual ~xGoal(){}

		virtual void activate() = 0;
		virtual WORD process() = 0;
		virtual void terminate() = 0;

		//virtual void addSubgoal(xGoal<entity_type>* g) {}

		inline bool isComplete()const{return status == Goal_completed;} 
		inline bool isActive()const{return status == Goal_active;}
		inline bool isInactive()const{return status == Goal_inactive;}
		inline bool hasFailed()const{return status == Goal_failed;}
		inline WORD getType()const{return type;}

	protected:

		WORD type;

		entity_type *owner;

		WORD status;	//GoalStateEnum

		void activateIfInactive() { if (isInactive()) activate(); }

		void reactivateIfFailed() { if (hasFailed())  status = Goal_inactive; }
};


#endif
