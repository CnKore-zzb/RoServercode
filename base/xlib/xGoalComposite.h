#ifndef _GOALCOMPOSITE_H_
#define _GOALCOMPOSITE_H_

#include <deque>
#include "xGoal.h"
#include "xTools.h"
//#include "xNewMem.h"

template <typename entity_type>
class xGoalComposite : public xGoal<entity_type>
{
	protected:

		WORD processSubgoals()
		{ 
			while (!subGoals.empty() && (subGoals[0]->isComplete() || subGoals[0]->hasFailed()))
			{    
				subGoals[0]->terminate();
				SAFE_DELETE(subGoals[0]); 
				subGoals.pop_front();
			}

			if (!subGoals.empty())
			{ 
				if (subGoals[0]->isInactive())
					subGoals[0]->activate();

				WORD statusOfSubGoals = subGoals[0]->process();

				if (statusOfSubGoals == Goal_completed)
					statusOfSubGoals = Goal_active;

				return statusOfSubGoals;
			}
			else
			{
				return Goal_completed;
			}
		}

		std::deque<xGoal<entity_type>* > subGoals;

	public:

		xGoalComposite(entity_type* pE, WORD type) : xGoal<entity_type>(pE, type){}

		virtual ~xGoalComposite() { removeAllSubGoals(); }

		inline void addSubGoal(xGoal<entity_type>* g) { subGoals.push_back(g); }
		inline DWORD getSubGoalSize() const { return subGoals.size(); }
		inline void clear() { removeAllSubGoals(); }

		void removeAllSubGoals()
		{
			for (WORD i = 0; i < subGoals.size(); ++i)
			{  
				subGoals[i]->terminate();
				SAFE_DELETE(subGoals[i]);
			}

			subGoals.clear();
		}
};

#endif

