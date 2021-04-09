/*
 * ni-scheduler.h
 *
 *  Created on: Mar 24, 2017
 *      Author: tbc
 */

#ifndef SRC_MACROFLOW_MODEL_NI_SCHEDULER_H_
#define SRC_MACROFLOW_MODEL_NI_SCHEDULER_H_


#include <vector>
#include <algorithm>

#include "ns3/ptr.h"
#include "ns3/simulator.h"
#include "ns3/bulk-send-application.h"

#include "ns3/macroflow.h"
#include "ns3/parallel-job.h"
#include "ns3/global-property.h"
#include "ns3/mf-scheduler.h"

namespace ns3 {

class NiScheduler {
public:
	enum Method { Fair = 0x0, Size = 0x1 };

	NiScheduler(uint32_t method = Fair);
	virtual ~NiScheduler();

	// call this function at the beginning of each step
	void CheckJobArrival(const std::vector<Ptr<ParallelJob> >& jobs);

	// get just finished macroflows and update the states of corresponding mfs/jobs
	std::vector<Ptr<Macroflow> > GetJustFinishedMfs();

	// whether running job table is empty
	bool HasWaitingJob() const;

	// get a reducer task to emit
	Ptr<Macroflow> GetReducerTask();

	bool IsAllFinished() const;

	const std::vector<Ptr<Macroflow> >& GetActiveMfList() const;


protected:
	uint32_t m_method;
	uint32_t m_noArrived;
	std::vector<Ptr<ParallelJob> > m_waitingJobs; // jobs that have not emit all reducers
	std::vector<Ptr<Macroflow> > m_activeMfs; // running macroflows

	static bool CompareFair(const Ptr<ParallelJob>& a, const Ptr<ParallelJob>& b);
	static bool CompareSize(const Ptr<ParallelJob>& a, const Ptr<ParallelJob>& b);
};

} /* namespace ns3 */

#endif /* SRC_MACROFLOW_MODEL_NI_SCHEDULER_H_ */
