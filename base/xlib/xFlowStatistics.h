#ifndef _X_FLOWSTATISTICS_H_
#define _X_FLOWSTATISTICS_H_

#include <map>
#include "xDefine.h"
#include "xLog.h"
#include "xTime.h"

class xFlowStatistics
{
	public:
		xFlowStatistics(const DWORD &interval) : log_timer(interval) {}
		~xFlowStatistics() {}

		inline void add(void* cmd, WORD len)
		{
			if (cmd && len)
			{
				stCmdCount &cc = flow[*((WORD*)cmd)];
				++cc.times;
				cc.size += len;
			}
		}
		inline void add(WORD len)
		{
			if (len)
			{
				stCmdCount &cc = flow[1];
				++cc.times;
				cc.size += len;
			}
		}
		void printLog(const bool &force_print = false);


	private:
		struct stCmdCount
		{
			DWORD times;
			QWORD size;
			stCmdCount() { bzero(this, sizeof(*this)); }
		};
		typedef std::map<WORD, stCmdCount> Flow;
		typedef Flow::iterator Flow_iter;
		Flow flow;

		xTimer log_timer;
};

#endif
