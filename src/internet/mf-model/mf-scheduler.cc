/*
 * mf-scheduler.cc
 *
 *  Created on: Mar 24, 2017
 *      Author: tbc
 */

#include "mf-scheduler.h"

namespace ns3 {

MfSchedulerSmf::MfSchedulerSmf(uint32_t algorithm) {
	m_algorithm = algorithm;
}

MfSchedulerSmf::~MfSchedulerSmf() { }

// call this function when trace file is just loaded
void
MfSchedulerSmf::CalcThresholds(const std::vector<Ptr<ParallelJob> >& jobs)
{
	// Z 依次取出传入的paralleljobs的mfs，然后把max(最大mf字节数，该mf字节数)加起来
	uint64_t maxMfBytes = 0;
	for(uint32_t i = 0;i<jobs.size();++i){
		const std::vector<Ptr<Macroflow> >& mfs = jobs[i]->GetMfList();
		for(uint32_t j = 0;j<mfs.size();++j)
			maxMfBytes = std::max(maxMfBytes,mfs[j]->GetBytes());
	}
	uint32_t noQueues = GlobalProperty::m_noQueues-1;

	// Z TODO 没看懂，似乎是一个什么指数算法，对给定队列数量的每一个优先级给出一个数字
	if(m_algorithm == Exp){
		double e = pow(static_cast<double>(maxMfBytes), 1.0/static_cast<double>(GlobalProperty::m_noQueues));
		m_thresTable[noQueues][0] = 1ULL;
		for(uint32_t prio = 1; prio<=noQueues; ++prio)
			m_thresTable[noQueues][prio] = static_cast<uint64_t>(m_thresTable[noQueues][prio-1]*e);
	}else{
		NS_FATAL_ERROR ("Unknown algorithm for SMF!");
	}
	m_thresTable[noQueues][0] = 0ULL;
	m_thresTable[noQueues][noQueues+1] = maxMfBytes;
}

// call this function in each simulation steps for each active mf
void
MfSchedulerSmf::SetPriority(Ptr<Macroflow> mf) const
{
	//modified by lkx
	uint32_t prio = 7;
	mf->SetPriority(prio);
	
	// Z 下面这里似乎是根据上一个函数计算的表格，来为传入的mf设定优先级
	/*uint32_t noQueues = GlobalProperty::m_noQueues-1;
	 uint64_t mfSize = mf->GetBytes();
	for(uint32_t prio = 0; prio<=noQueues; ++prio){ // note that noQueues has been decreased so we use <=
		if(mfSize<=m_thresTable[noQueues][prio+1]){
			mf->SetPriority(prio);
			break;
		}
	}*/
}

} /* namespace ns3 */
