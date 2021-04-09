/*
 * mf-scheduler.h
 *
 *  Created on: Mar 24, 2017
 *      Author: tbc
 */

#ifndef SRC_MACROFLOW_MODEL_MF_SCHEDULER_H_
#define SRC_MACROFLOW_MODEL_MF_SCHEDULER_H_


#include <vector>

#include "ns3/ptr.h"

#include "ns3/parallel-job.h"
#include "ns3/macroflow.h"


namespace ns3 {

// focus on scheduling algorithm only
class MfScheduler {
public:
	MfScheduler() {};
	virtual ~MfScheduler() {};

	// call this function when trace file is just loaded
	virtual void CalcThresholds(const std::vector<Ptr<ParallelJob> >& jobs) = 0;

	// call this function in each simulation steps for each active job
	virtual void SetPriority(Ptr<Macroflow> mf) const = 0;

};

class MfSchedulerSmf: public MfScheduler {
public:

	enum{Exp=0};

	MfSchedulerSmf(uint32_t algorithm = Exp);
	virtual ~MfSchedulerSmf();

	// call this function when trace file is just loaded
	virtual void CalcThresholds(const std::vector<Ptr<ParallelJob> >& jobs);

	// call this function in each simulation steps for each active job
	virtual void SetPriority(Ptr<Macroflow> mf) const;

protected:
	uint32_t m_algorithm;
	uint64_t m_thresTable[8][9];
};


} /* namespace ns3 */

#endif /* SRC_MACROFLOW_MODEL_MF_SCHEDULER_H_ */
