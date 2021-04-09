/*
 * ci-scheduler.cc
 *
 *  Created on: Mar 25, 2017
 *      Author: tbc
 */

#include "ci-scheduler.h"

namespace ns3 {

// Z 大概是Computation-intensive的意思，即计算密集型，作为背景流？
// Z 项目中是none
CiScheduler::CiScheduler(uint32_t method) {
	if(method!=Oppo && method!=Peri && method!=None)
	// Z 机会主义 周期性的
		NS_FATAL_ERROR ("Only opportunistic and periodic is supported in ci-job scheduling!");
	//if(method!=Oppo)
	//	NS_FATAL_ERROR ("Currently we only support opportunistic method in ci-job scheduling!");

	m_method = method;
	m_noArrived = 0;
	m_lastArrivalTime = 0;
}

CiScheduler::~CiScheduler() {
	// TODO Auto-generated destructor stub
}


// call this function at the beginning of each step
void
CiScheduler::CheckJobArrival()
{
	if(m_method == Oppo || m_method == None){
		// do nothing
	} else {
		// Z 只有周期性模式下运行这段代码
		uint64_t now = Simulator::Now().GetMilliSeconds();
		// Z 如果现在的时间和上一次到达的时间的差小于设定的间隔直接返回
		if(now - m_lastArrivalTime < GlobalProperty::m_periods) return;
		// Z 如果已经到达的包的数量大于设定的总数？
		if(m_noArrived >= GlobalProperty::m_totalCi) return;

		Log2("[Ci-Arrived]  Id: %u  Time: %u", m_noArrived, static_cast<uint32_t>(now));
		Ptr<ParallelJob> job = Create<ParallelJob>();
		job->Initialize(m_noArrived++, ParallelJob::CiJob);
		job->InitializeCiProperty(GlobalProperty::m_ciDuration, Simulator::Now());
		job->SetCiDuration(GlobalProperty::m_ciDuration);
		job->SetJobState(ParallelJob::Pending);
		m_waitingJobs.push(job);
		m_lastArrivalTime = now;
	}
}


std::vector<Ptr<ParallelJob> >
CiScheduler::GetJustFinishedJobs()
{
	std::vector<Ptr<ParallelJob> > finishedJobs;
	do{
		if(m_activeJobs.empty())
			break;

		Ptr<ParallelJob> job = m_activeJobs.front();
		Time diff = TimeDiff(Simulator::Now(),job->GetArrivalTime());
		int compare = diff.Compare(job->GetCiDuration());
		if(compare<0) // considering the time-monotonicity of m_activeJobs
			break;

		// deal with finished ci-jobs: modify the properties
		m_activeJobs.pop();
		job->SetFinishTime(Simulator::Now());
		job->SetJobState(ParallelJob::Finished);
		finishedJobs.push_back(job);
		Log2("[Ci-Finished]  Id: %u  Time: %u", job->GetId(), static_cast<uint32_t>(Simulator::Now().GetMilliSeconds()));
		m_time.push_back( job->GetFinishTime().GetMilliSeconds() - job->GetArrivalTime().GetMilliSeconds() );
	}while(true);
	return finishedJobs;
}

// whether running job table is empty
bool
CiScheduler::HasActiveJob() const
{
	// 1st condition: no running jobs
	// 2nd condition: all ni-jobs has emitted and thus, we stop emit ci-jobs
	if(m_method == Oppo){
		return GlobalProperty::m_noEmittedNiJobs < GlobalProperty::m_noTotalNiJobs || !m_activeJobs.empty();
	} else if(m_method == Peri){
		return m_noArrived<GlobalProperty::m_totalCi || !m_activeJobs.empty();
	} else{
		return false;
	}
}

// can we get a job instantly?
bool
CiScheduler::HasWaitingJob() const
{
	if(m_method == Oppo){
		return GlobalProperty::m_noEmittedNiJobs < GlobalProperty::m_noTotalNiJobs;
	} else if(m_method == Peri){
		return !m_waitingJobs.empty();
	} else{
		return false;
	}
}

bool
CiScheduler::HasPendingJob() const
{
	if(m_method == Oppo || m_method==None){
		return false;
	} else {
		return !m_waitingJobs.empty();;
	}
}

// get a reducer task to emit
Ptr<ParallelJob>
CiScheduler::GetComputationTask()
{
	if(m_method == Oppo){
		Log2("[Ci-Arrived]  Id: %u  Time: %u", m_noArrived, static_cast<uint32_t>(Simulator::Now().GetMilliSeconds()));
		Ptr<ParallelJob> job = Create<ParallelJob>();
		job->Initialize(m_noArrived++, ParallelJob::CiJob);
		job->InitializeCiProperty(GlobalProperty::m_ciDuration, Simulator::Now());
		job->SetStartTime(Simulator::Now());
		job->SetCiDuration(GlobalProperty::m_ciDuration);
		job->SetJobState(ParallelJob::Active);
		m_activeJobs.push(job);
		m_lastArrivalTime = Simulator::Now().GetMilliSeconds();
		return job;
	} else if(m_method == Peri){
		Ptr<ParallelJob> job = m_waitingJobs.front();
		m_waitingJobs.pop();
		job->SetStartTime(Simulator::Now());
		job->SetJobState(ParallelJob::Active);
		m_activeJobs.push(job);
		return job;
	} else{
		return NULL;
	}
}

bool
CiScheduler::IsAllFinished() const
{
	return !HasActiveJob() && !HasPendingJob();
}

const std::vector<uint64_t>&
CiScheduler::GetTimeList() const
{
	return m_time;
}

uint64_t
CiScheduler::GetLastArraivalTime() const
{
	return m_lastArrivalTime;
}


} /* namespace ns3 */
