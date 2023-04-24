#include "xFlowStatistics.h"

void xFlowStatistics::printLog(const bool &force_print)
{
	if ((force_print || log_timer.timeUp(now())) && !flow.empty())
	{
		QWORD totalSize = 0;
		QWORD totalTimes = 0;
		WORD cmd = 0;
		WORD param = 0;
		XDBG << "[流量统计]" << (force_print ? "强制" : "时间") << ",间隔:" << log_timer.getElapse() << ",数量:" << (DWORD)flow.size() << XEND;
		std::map<DWORD, WORD> timeslist;
		std::map<QWORD, WORD> sizelist;
		timeslist.clear();
		sizelist.clear();
		for (Flow_iter iter = flow.begin(); iter != flow.end(); ++iter)
		{
			XLOG << "[流量统计],消息(" << (iter->first & 0xff) << "," << ((iter->first >> 8) & 0xff) << "),大小:" << iter->second.size << ",次数:" << iter->second.times << ",平均:" << iter->second.size / iter->second.times << XEND; 

			timeslist[iter->second.times] = iter->first;
			sizelist[iter->second.size] = iter->first;

			totalSize += iter->second.size;
			totalTimes += iter->second.times;
		}
		flow.clear();
		XDBG << "[流量统计]总大小:" << totalSize << ",总次数:" << totalTimes << ",平均:" << totalSize / totalTimes << XEND;

		for (std::map<DWORD, WORD>::iterator iter=timeslist.begin(); iter!=timeslist.end(); iter++)
		{
			cmd = (iter->second & 0xff);
			param = ((iter->second >> 8) & 0xff);
			XLOG << "[流量次数统计],消息(" << cmd << "," << param << "),次数:" << iter->first << XEND;
		}
		for (std::map<QWORD, WORD>::iterator iter=sizelist.begin(); iter!=sizelist.end(); iter++)
		{
			cmd = (iter->second & 0xff);
			param = ((iter->second >> 8) & 0xff);
			XLOG << "[流量大小统计],消息(" << cmd << "," << param << "),大小:" << iter->first << XEND;
		}
	}
}

