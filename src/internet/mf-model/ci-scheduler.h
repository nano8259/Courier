/*
 * ci-scheduler.h
 *
 *  Created on: Mar 25, 2017
 *      Author: tbc
 */

#ifndef SRC_MACROFLOW_MODEL_CI_SCHEDULER_H_
#define SRC_MACROFLOW_MODEL_CI_SCHEDULER_H_

#include <queue>
#include <vector>

#include "ns3/ptr.h"
#include "ns3/simulator.h"

#include "ns3/global-property.h"
#include "ns3/parallel-job.h"

namespace ns3 {

class CiScheduler {
public:
	// Z TODO ci代表什么，Oppo和Peri代表什么
	enum Method { Oppo = 0x0, Peri = 0x1, None = 0x2 };

	CiScheduler(uint32_t method = Oppo);
	virtual ~CiScheduler();

	// call this function at the beginning of each step
	void CheckJobArrival();

	// Z TODO 看Paralleljob
	std::vector<Ptr<ParallelJob> > GetJustFinishedJobs();

	// whether active job table is empty
	bool HasActiveJob() const;

	// can we get a job instantly?
	bool HasWaitingJob() const;

	// whether there is un-arrived ci-jobs
	bool HasPendingJob() const;

	// get a reducer task to emit
	Ptr<ParallelJob> GetComputationTask();

	bool IsAllFinished() const;

	const std::vector<uint64_t>& GetTimeList() const;
	uint64_t GetLastArraivalTime() const;

protected:
	uint32_t m_method;

	uint32_t m_noArrived;
	std::vector<uint64_t> m_time;
	std::queue<Ptr<ParallelJob> > m_activeJobs;
	uint64_t m_lastArrivalTime;

	std::queue<Ptr<ParallelJob> > m_waitingJobs;
	uint32_t m_totalArrivedCi;


};

} /* namespace ns3 */

#endif /* SRC_MACROFLOW_MODEL_CI_SCHEDULER_H_ */
