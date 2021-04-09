/*
 * parallel-job.h
 *
 *  Created on: Mar 21, 2017
 *      Author: tbc
 */

#ifndef SRC_MACROFLOW_UTIL_PARALLEL_JOB_H_
#define SRC_MACROFLOW_UTIL_PARALLEL_JOB_H_

#include <vector>

#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "ns3/object.h"

#include "ns3/global-property.h"
#include "ns3/macroflow.h"

namespace ns3 {

//class Macroflow;

class ParallelJob : public Object{
public:
	ParallelJob();
	//virtual ~ParallelJob() {}

	enum JobType { CiJob = 0x01, NiJob = 0x02}; // Ci-01B Ni-10B
	enum JobState { Pending = 0u, Active = 1u, Finished = 2u };

	void Initialize(uint32_t jobId, uint8_t jobType);
	void InitializeCiProperty(Time ciDuration, Time arrivalTime);
	void InitializeNiProperty(uint32_t noMappers, uint32_t noReducers, Time arrivalTime, std::vector<uint32_t> originalMapperMachine);
	void AddMacroflow(Ptr<Macroflow> mf);

	/*uint32_t GetId() const;
	uint8_t GetJobType() const;

	const Time& GetArrivalTime() const;
	void SetStartTime(const Time& startTime);
	void SetFinishTime(const Time& finishTime);
	const Time& GetStartTime() const;
	const Time& GetFinishTime() const;

	void SetJobState(uint32_t state);
	uint32_t GetJobState() const;

	uint32_t GetActiveMfs() const;
	uint32_t GetEmittedMfs() const;
	uint32_t GetFinishedMfs() const;
	void SetActiveMfs(uint32_t activeMfs);
	void SetEmittedMfs(uint32_t emittedMfs);
	void SetFinishedMfs(uint32_t finishedMfs);

	uint32_t GetReducers() const;
	uint64_t GetTotalBytes() const;
	const std::vector<Ptr<Macroflow> >& GetMfList() const;
	const std::vector<uint32_t>& GetMapperMachineList() const;

	uint32_t GetNoMappers() const;
	uint32_t GetNoReducers() const;

	const Time& GetCiDuration() const;
	void SetCiDuration(const Time& ciDuration);

	uint32_t GetCiMachine() const;
	void setCiMachine(uint32_t machine);*/

	inline uint32_t GetId() const { return m_jobId; }
	inline uint8_t GetJobType() const { return m_jobType; }
	inline const Time& GetArrivalTime() const { return m_arrivalTime; }
	inline void SetStartTime(const Time& startTime) { m_startTime = startTime; }
	inline void SetFinishTime(const Time& finishTime) { m_finishTime = finishTime; }
	inline const Time& GetStartTime() const { return m_startTime; }
	inline const Time& GetFinishTime() const { return m_finishTime; }
	inline void SetJobState(uint32_t state) { m_state = state; }
	inline uint32_t GetJobState() const { return m_state; }
	inline uint32_t GetActiveMfs() const { return m_noActiveMfs; }
	inline uint32_t GetEmittedMfs() const { return m_noEmittedMfs; }
	inline uint32_t GetFinishedMfs() const { return m_noFinishedMfs; }
	inline void SetActiveMfs(uint32_t activeMfs) { m_noActiveMfs = activeMfs; }
	inline void SetEmittedMfs(uint32_t emittedMfs) { m_noEmittedMfs = emittedMfs; }
	inline void SetFinishedMfs(uint32_t finishedMfs) { m_noFinishedMfs = finishedMfs; }
	inline uint32_t GetReducers() const { return m_noReducers; }
	inline uint64_t GetTotalBytes() const { return m_totalBytes; }
	inline const std::vector<Ptr<Macroflow> >& GetMfList() const { return m_macroflowList; }
	inline const std::vector<uint32_t>& GetMapperMachineList() const { return m_mapperMachines; }
	inline uint32_t GetNoMappers() const { return m_noMappers; }
	inline uint32_t GetNoReducers() const { return m_noReducers; }
	inline const Time& GetCiDuration() const { return m_ciDuration; }
	inline void SetCiDuration(const Time& ciDuration) { m_ciDuration = ciDuration; }
	inline uint32_t GetCiMachine() const { return m_ciMachine; }
	inline void setCiMachine(uint32_t machine) { m_ciMachine = machine; }

	inline uint64_t GetSentBytes(){
		if(Simulator::Now() != m_sentBytesCalcTime){
			m_sentBytes = 0;
			for(uint32_t i=0;i<m_macroflowList.size();++i)
				m_sentBytes += m_macroflowList[i]->GetSentBytes();
			m_sentBytesCalcTime = Simulator::Now();
		}
		return m_sentBytes;
	}

protected:
	bool m_initialzed;

	// set by constructor
	uint32_t m_jobId;
	uint8_t m_jobType;
	uint32_t m_state;

	// determined by traces or fixed settings
	uint32_t m_noMappers;
	uint32_t m_noReducers;
	uint64_t m_totalBytes;
	Time m_ciDuration; // for cijob only
	Time m_arrivalTime;

	uint32_t m_ciMachine; // for cijob only

	std::vector<Ptr<Macroflow> > m_macroflowList;
	std::vector<uint32_t> m_mapperMachines;
	std::vector<uint32_t> m_reducerMachines;

	// dynamic
	uint32_t m_noActiveMfs;
	uint32_t m_noEmittedMfs;
	uint32_t m_noFinishedMfs;

	Time m_startTime;
	Time m_finishTime;

	Time m_sentBytesCalcTime;
	uint64_t m_sentBytes;

};

} /* namespace ns3 */

#endif /* SRC_MACROFLOW_UTIL_PARALLEL_JOB_H_ */
