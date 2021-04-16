/*
 * macroflow.h
 *
 *  Created on: Mar 21, 2017
 *      Author: tbc
 */

#ifndef SRC_MACROFLOW_UTIL_MACROFLOW_H_
#define SRC_MACROFLOW_UTIL_MACROFLOW_H_

#include <vector>
#include <string>
#include <algorithm>
#include <set>
#include <map>

#include "ns3/ptr.h"
#include "ns3/ipv4-header.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/application.h"
#include "ns3/bulk-send-application.h"
#include "ns3/simulator.h"

#include "ns3/tcp-flow-helper.h"
//#include "ns3/tracker.h"

namespace ns3 {

// AggregatedMapper: given a reducer, all of its mappers from a certain host are regarded as one aggregated mapper
class AggregatedMapper{
public:
	enum FlowState { Pending = 0u, Paused = 1u, Started = 2u, Finished = 3u };
	uint32_t m_machineId;
	uint64_t m_size;
	FlowState m_state;
	Ptr<BulkSendApplication> m_flow;
	AggregatedMapper(uint32_t machineId, uint64_t size, FlowState state, void* flow = NULL){
		m_machineId=machineId; m_size=size; m_state=state;
		m_flow = Ptr<BulkSendApplication>((BulkSendApplication*)flow);
	}
};


class Macroflow : public Object{
public:
	enum MfState { Pending = 0u, Active = 1u, ShuffleFinished = 2u, Finished = 3u };

	Macroflow();
	virtual ~Macroflow() {}

	void Initialize(uint32_t macroflowId, Ptr<Object> job, uint64_t totalBytes);
	void SetMapperList(const std::vector<uint32_t>& mapperList);

	inline void SetPriority(uint32_t priority) { m_priority = priority; }

	inline uint8_t GetDscp() const
	{
		static uint8_t dscpTable[] = {
				Ipv4Header::DscpDefault,
				Ipv4Header::CS1,
				Ipv4Header::CS2,
				Ipv4Header::CS3,
				Ipv4Header::CS4,
				Ipv4Header::CS5,
				Ipv4Header::CS6,
				Ipv4Header::CS7
		};
		return dscpTable[7-m_priority];
	}

	inline std::string GetDscpString() const { std::string s = "0"; s[0] += 7-m_priority; return "CS"+s; }

	inline uint64_t GetBytes() const { return m_totalBytes; }

	inline Ptr<Object> GetJob() const { return m_job; }

	inline uint32_t GetId() const { return m_macroflowId; }

	inline void SetStartTime(const Time& startTime) { m_startTime = startTime; }

	inline void SetFinishTime(const Time& finishTime) { m_finishTime = finishTime; }

	inline void SetShuffleFinishTime(const Time& shuffleFinishTime) { m_shuffleFinishTime = shuffleFinishTime; }

	inline const Time& GetStartTime() const { return m_startTime; }

	inline const Time& GetFinishTime() const { return m_finishTime; }

	inline const Time& GetShuffleFinishTime() const { return m_shuffleFinishTime; }

	/*inline void AddFlow(Ptr<BulkSendApplication> flow) {
		uint32_t machineId = flow->GetMapperHostNumber();
		uint32_t machinePos = (uint32_t)(-1);
		for(uint32_t i=0;i<m_mapperHostList.size();++i){
			if(m_mapperHostList[i].m_machineId == machineId) { machinePos=i; break; }
		}
		m_mapperHostList[machinePos].m_flow = flow;
	}*/

	inline const std::vector<AggregatedMapper>& GetFlows() const { return m_mapperHostList; }

	inline void SetState(uint32_t state) { m_state = state; }

	inline uint32_t GetState() const { return m_state; }

	inline void SetReducerMachine(uint32_t reducerMachine) { m_reducerMachine = reducerMachine; }

	inline uint32_t GetReducerMachine() const { return m_reducerMachine; }

	inline uint64_t GetSentBytes(){
		if(Simulator::Now() != m_sentBytesCalcTime){
			m_sentBytes = 0;
			for(uint32_t i=0;i<m_mapperHostList.size();++i){
				if(m_mapperHostList[i].m_state == AggregatedMapper::Pending) continue;
				m_sentBytes += m_mapperHostList[i].m_flow->GetMfSentBytes(m_macroflowId);
			}
			m_sentBytesCalcTime = Simulator::Now();
		}
		return m_sentBytes;
	}

	inline void SetEstimatedMct(double estimatedMct) { m_estimatedMct = estimatedMct; }

	inline double GetEstimatedMct() const { return m_estimatedMct; }

	void RemoveFinishedFlows();


	inline const std::set<uint32_t>& GetActiveHosts() const { return m_activeHosts; }
	inline uint32_t GetFinishedFlowsNumber() const {return m_finishedFlows;}

	// called by MachineSlots::ReallocateFetchers()
	inline uint32_t GetMaxConnection() const {return m_maxConnection;}
	inline void SetMaxConnection(uint32_t maxConnection) {
		m_maxConnection = maxConnection;
		if(GlobalProperty::m_bwAlgorithm)adjustConnectionBw();
		// else adjustConnection();
	}

	inline void SetPriority(bool prioritized){m_prioritized =prioritized; }

	// initialized by TraceReader::Analysis()
	static void (*ReportMapperList)(Ptr<Macroflow>);
	static std::vector<uint32_t> (*GetCandidateList)(Ptr<Macroflow>,bool);
	static void (*ReportChosenHosts)(Ptr<Macroflow>, const std::vector<uint32_t>&);
	static void (*ReportPausedHosts)(Ptr<Macroflow>, const std::vector<uint32_t>&);

	// added by zzc
	void SetFlows(std::map<uint32_t, Ptr<BulkSendApplication>> fs);

protected:

	// if there are more free connection resources given, we generate more flows;
	// otherwise, we stop some flows
	// void adjustConnection();
	void adjustConnectionBw();

	bool m_initialzed;
	uint32_t m_state;

	// set by constructor
	uint32_t m_macroflowId;
	Ptr<Object> m_job; // Ptr<ParallelJob>

	// determined by traces or fixed settings
	uint64_t m_totalBytes;

	Time m_startTime;
	Time m_shuffleFinishTime;
	Time m_finishTime;

	uint32_t m_priority; // high->low: 0 ~ noQueues-1
	uint32_t m_reducerMachine;

	//std::vector<Ptr<Application> > m_flows;
	uint32_t m_finishedFlows;

	Time m_sentBytesCalcTime;
	uint64_t m_sentBytes;

	double m_estimatedMct;

	// fetcher

	// uint32_t m_nextMapperHostIdx;// the next mapper-host that will become active
	// std::vector<std::vector<FlowState> > m_flowState; //f[i][j] is the flow state of the j-th mapper in the i-th machine for current reducer

	// uint64_t m_szFinishedFlows; // the total size of finished flows

	// eurosys
	uint32_t m_maxConnection;
	std::vector<AggregatedMapper> m_mapperHostList; // <the mapper-host number, bytes> list, in random order
	std::set<uint32_t> m_activeHosts; // hosts number that in active fetcher list
	bool m_prioritized;

	// added by zzc
	std::map<uint32_t, Ptr<BulkSendApplication>> m_flows;

};

} /* namespace ns3 */

#endif /* SRC_MACROFLOW_UTIL_MACROFLOW_H_ */
