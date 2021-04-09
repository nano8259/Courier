/*
 * ni-scheduler.cc
 *
 *  Created on: Mar 24, 2017
 *      Author: tbc
 */

#include "ni-scheduler.h"

namespace ns3 {

// Z 项目中method是Fair
NiScheduler::NiScheduler(uint32_t method) {
	if(method!=Fair && method!=Size)
		NS_FATAL_ERROR ("Only fair-sharing and smallest-job-first is supported in ni-job scheduling!");

	m_method = method;
	m_noArrived = 0;
}

NiScheduler::~NiScheduler() { }

// call this function at the beginning of each step
void
NiScheduler::CheckJobArrival(const std::vector<Ptr<ParallelJob> >& jobs)
{
	Time now = Simulator::Now();
	// 在调用的时候，若时间超过或等于一些job的到达时间，启动这些job
	while(m_noArrived<jobs.size() && now.Compare(jobs[m_noArrived]->GetArrivalTime())>=0){
		if(GlobalProperty::m_log_state) Log2("[Ni-Arrived]  Id: %u  Time: %u", m_noArrived, static_cast<uint32_t>(now.GetMilliSeconds()));//lkx
		jobs[m_noArrived]->SetStartTime(now);
		jobs[m_noArrived]->SetJobState(ParallelJob::Active);
		m_waitingJobs.push_back(jobs[m_noArrived++]);
	}
	// XXX we do not keep the ascending order of m_activeJobs here, we sort it before reading elements
}

std::vector<Ptr<Macroflow> >
NiScheduler::GetJustFinishedMfs()
{
	std::vector<Ptr<Macroflow> > finishedMfs;

	uint32_t i=0;
	while(i<m_activeMfs.size()){
		Ptr<Macroflow> mf = m_activeMfs[i];
		//const std::vector<Ptr<Application> >& flows = mf->GetFlows();
		ParallelJob* job = static_cast<ParallelJob*>(mf->GetJob().operator ->());
		bool mfFinished = mf->GetFinishedFlowsNumber()==mf->GetFlows().size(); // bug fixed, caused by aggregated mapper
		/*for(uint32_t j=0; j<flows.size(); ++j){
			BulkSendApplication* flow = static_cast<BulkSendApplication*>(flows[j].operator ->());
			if(!flow->IsFinished()){
				mfFinished = false;
				break;
			}
		}*/
		mf->RemoveFinishedFlows();

		//if not finished, check the next mf, else add it to the finishMfs vector...
		if(!mfFinished){
			++i;
			continue; // network phase has not finished, continue waiting
		}

		// here the macroflow has finished its network phase

		// check whether it has computation phase
		uint8_t hasComputationPhase = ( job->GetJobType() & ParallelJob::CiJob );
		if(hasComputationPhase){
			uint32_t state = mf->GetState();
			// Z 设置shuffle结束时间
			if(state == Macroflow::Active){ // network phase just finished
				mf->SetState(Macroflow::ShuffleFinished);
				mf->SetShuffleFinishTime(Simulator::Now());
				if(GlobalProperty::m_log_state) Log3("[Mf-ShuffleFinished]  Id: %u  Time: %u  JobId: %u", mf->GetId(), static_cast<uint32_t>(Simulator::Now().GetMilliSeconds()), job->GetId());
			}else if(state == Macroflow::ShuffleFinished){
				// do nothing
			}else{
				NS_FATAL_ERROR ("Bug: a finished macroflow still in active-mf-queue!");
			}
			Time finishTime = mf->GetShuffleFinishTime() + job->GetCiDuration();
			if(finishTime.Compare(Simulator::Now())>0){
				++i;
				continue; // computation phase has not finished, continue waiting
			}
		}else{
			mf->SetShuffleFinishTime(Simulator::Now()); // shuffle finish time == finish time
		}

		// here the macroflow has finished all its works

		// update job/mf states
		mf->SetState(Macroflow::Finished);
		mf->SetFinishTime(Simulator::Now());
		uint32_t noFinishedMfs = job->GetFinishedMfs()+1;
		job->SetFinishedMfs(noFinishedMfs);
		uint32_t noActiveMfs = job->GetActiveMfs()-1;
		job->SetActiveMfs(noActiveMfs);
		if(GlobalProperty::m_log_state) Log3("[Mf-Finished]  Id: %u  Time: %u  JobId: %u", mf->GetId(), static_cast<uint32_t>(Simulator::Now().GetMilliSeconds()), job->GetId());//lkx

		//the job(coflow) is finished when all the macroflow are finished
		if(noFinishedMfs == job->GetNoReducers()){
			//Log2("[Ni-Finished]  Id: %u  Time: %u", job->GetId(), static_cast<uint32_t>(Simulator::Now().GetMilliSeconds()));//lkx
			job->SetJobState(ParallelJob::Finished);
			job->SetFinishTime(Simulator::Now());
			++GlobalProperty::m_noFinishedNiJobs;
		}

		// update running-mf lists
		finishedMfs.push_back(mf);
		std::vector<Ptr<Macroflow> >::iterator it = m_activeMfs.begin() + i;
		m_activeMfs.erase(it); // note that we do not increase the variable $i$, due to the erasion of $it$
	}

	return finishedMfs;

}

// whether running job table is empty
bool
NiScheduler::HasWaitingJob() const
{
	return !m_waitingJobs.empty();
}

// get a reducer task to emit, and update the properties of corresponding job and mf
Ptr<Macroflow>
NiScheduler::GetReducerTask()
{
	if(m_waitingJobs.empty())
		NS_FATAL_ERROR("No active ni-job!");
	if(m_method == Fair)
		std::sort(m_waitingJobs.begin(), m_waitingJobs.end(), &NiScheduler::CompareFair);
	else // method == Size
		std::sort(m_waitingJobs.begin(), m_waitingJobs.end(), &NiScheduler::CompareSize);

	Ptr<ParallelJob> job = m_waitingJobs[0];
	uint32_t noEmittedMfs =job->GetEmittedMfs();
	Ptr<Macroflow> mf = (job->GetMfList())[noEmittedMfs];
    
	if(GlobalProperty::m_log_state) Log3("[Mf-Started]  Id: %u  Time: %u  JobId: %u", noEmittedMfs, static_cast<uint32_t>(Simulator::Now().GetMilliSeconds()),job->GetId() );//lkx
	mf->SetStartTime(Simulator::Now());
	mf->SetState(Macroflow::Active);
	job->SetEmittedMfs(noEmittedMfs+1);
	job->SetActiveMfs(job->GetActiveMfs()+1);
	// the macroflow in the job are all emitted
	if(noEmittedMfs+1 == job->GetReducers()){
		m_waitingJobs.erase(m_waitingJobs.begin()); // pop the front element
		++GlobalProperty::m_noEmittedNiJobs; // update emitted jobs
	}
	m_activeMfs.push_back(mf);

	return mf;
}

bool
NiScheduler::IsAllFinished() const
{
	return GlobalProperty::m_noFinishedNiJobs == GlobalProperty::m_noTotalNiJobs;
}

const std::vector<Ptr<Macroflow> >&
NiScheduler::GetActiveMfList() const
{
	return m_activeMfs;
}

bool
NiScheduler::CompareFair(const Ptr<ParallelJob>& a, const Ptr<ParallelJob>& b)
{
	return a->GetActiveMfs() < b->GetActiveMfs();
}

bool
NiScheduler::CompareSize(const Ptr<ParallelJob>& a, const Ptr<ParallelJob>& b)
{
	return a->GetTotalBytes() < b->GetTotalBytes();
}


} /* namespace ns3 */
