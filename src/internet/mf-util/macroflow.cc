/*
 * macroflow.cc
 *
 *  Created on: Mar 21, 2017
 *      Author: tbc
 */

#include "macroflow.h"

namespace ns3 {

// initialized by TraceReader::Analysis()
void (*Macroflow::ReportMapperList)(Ptr<Macroflow>);
std::vector<uint32_t> (*Macroflow::GetCandidateList)(Ptr<Macroflow>,bool) = NULL;
void (*Macroflow::ReportChosenHosts)(Ptr<Macroflow>, const std::vector<uint32_t>&) = NULL;
void (*Macroflow::ReportPausedHosts)(Ptr<Macroflow>, const std::vector<uint32_t>&);

// note that this function may be warned by IDE due to lack of variable initialization, however, we don't care
Macroflow::Macroflow() {
	m_initialzed = false;
}

void
Macroflow::Initialize(uint32_t macroflowId, Ptr<Object> job, uint64_t totalBytes)
{
	if(m_initialzed)
		NS_FATAL_ERROR ("Macroflow has been initialized!");
	m_initialzed = true;

	m_macroflowId = macroflowId;
	m_job = job;
	m_totalBytes = totalBytes;
	m_priority = 0;
	m_finishedFlows = 0;
	m_state = Pending;
	m_reducerMachine = static_cast<uint32_t>(-1);
	m_prioritized = true;

	m_estimatedMct = 1; // use 1 as initial MCT
}

void Macroflow::SetMapperList(const std::vector<uint32_t>& mapperList)
{
	const uint32_t mx = 256; // max number of machine
	uint32_t count[mx] = {0};
	for(uint32_t i = 0;i<mapperList.size();++i)
		++count[mapperList[i]];
	// Z 这个flowsize应该是有一个假设：从每个mapper接收的大小都一样
	uint64_t flowSize = this->GetBytes() / mapperList.size();
	for(uint32_t i = 0;i<mx;++i)
		// Z 一个机器上的mappers视为一个AggregatedMapper
		if(count[i]) {
			// m_flows[i]->AddMf(m_macroflowId, count[i]*flowSize);
			m_mapperHostList.push_back(AggregatedMapper(i, count[i]*flowSize, AggregatedMapper::Pending));//count * flowsize: the same host can have several mapper
		}
	// Z 下面这个没看懂，把舍入误差加回去应该是
	if(!m_mapperHostList.empty()) // guarantee that the sum of flow size = reducer size
		m_mapperHostList[0].m_size += this->GetBytes()-flowSize*mapperList.size();

	std::random_shuffle(m_mapperHostList.begin(),m_mapperHostList.end());
	// m_nextMapperHostIdx = 0;
}

void Macroflow::SetFlows(std::map<uint32_t, Ptr<BulkSendApplication>> fs){
	m_flows = fs;
	std::vector<AggregatedMapper>::iterator it;
	for(it = m_mapperHostList.begin(); it != m_mapperHostList.end();){
		if(it -> m_machineId != this->GetReducerMachine()){
			it->m_flow = m_flows[it->m_machineId];
			m_flows[it->m_machineId]->AddMf(m_macroflowId, it->m_size);
			m_flows[it->m_machineId]->Continue();
			it++;
		}else{
			it->m_state=AggregatedMapper::Finished; ++m_finishedFlows;
			// create a local flow and set some property
			it->m_flow = Create<BulkSendApplication>();
			it->m_flow->AssumeFinished(it->m_size);
			it->m_flow->SetMapperHostNumber(it->m_machineId);
			m_mapperHostList.erase(it);
		}
	}
}

//Algorithm2...
void Macroflow::adjustConnectionBw() {
	if(GlobalProperty::m_courier_mode != GlobalProperty::CourierMode::schedule)
		return;
	// std::cout << "<Macroflow> adjustConnectionBw 0" << std::endl;	
	this->RemoveFinishedFlows();
	// std::cout << "<Macroflow> adjustConnectionBw 1" << std::endl;
	// check local flow
	//set the local flows to finished state?...
	//if(m_prioritized) std::cout<<"prioritized "<<this->m_maxConnection<<std::endl;
	//else std::cout<<this->m_maxConnection<<std::endl;

	// stop some flows
	std::vector<uint32_t> pausedHost;
	while(m_activeHosts.size() > m_maxConnection){
		//std::cout<<"testing!"<<std::endl;
		std::set<uint32_t>::iterator chosenIt;
		uint32_t chosenMachinePos = -1; // fastest flow
		bool flag = false;
		for(std::set<uint32_t>::iterator it = m_activeHosts.begin();it!=m_activeHosts.end();++it){
			// std::cout << "<Macroflow> adjustConnectionBw 2" << std::endl;
			uint32_t machinePos = -1;
			// Z 找machinePos
			for(uint32_t i = 0;i<m_mapperHostList.size();++i){
				if(m_mapperHostList[i].m_machineId==*it){ machinePos = i; break; }
			}
			//if(chosenMachinePos == (uint32_t)-1 ||
			// Z flag==false是为了让第一个能选上
			if(flag == false ||
					m_mapperHostList[chosenMachinePos].m_flow->GetMfCurrentSpeed(m_macroflowId) < m_mapperHostList[machinePos].m_flow->GetMfCurrentSpeed(m_macroflowId)){
				chosenMachinePos = machinePos; chosenIt = it; flag = true;
			}
		}
		// std::cout << "<Macroflow> adjustConnectionBw 3" << std::endl;
		m_activeHosts.erase(chosenIt);
		Ptr<BulkSendApplication> flow = m_mapperHostList[chosenMachinePos].m_flow;
		flow->RemovePrioritizedMf(m_macroflowId);
		//std::cout<<"test"<<std::endl;
		if(flow->IsMfFinished(m_macroflowId)){ // we don't use flow->IsFinished() here
			// m_mapperHostList[chosenMachinePos].m_state = AggregatedMapper::Finished;
			// m_mapperHostList[chosenMachinePos].m_size = 0;
			// ++m_finishedFlows;
		}else{
			// Z Pending..为啥不是Paused
			m_mapperHostList[chosenMachinePos].m_state = AggregatedMapper::Pending;
			m_mapperHostList[chosenMachinePos].m_size = flow->GetMfFlowSize(m_macroflowId) - flow->GetMfSentBytes(m_macroflowId);
			// m_mapperHostList[chosenMachinePos].m_flow = Ptr<BulkSendApplication>();
			pausedHost.push_back(m_mapperHostList[chosenMachinePos].m_machineId);
		}
	}
	if(!pausedHost.empty()) Macroflow::ReportPausedHosts(this, pausedHost);
	// std::cout << "<Macroflow> adjustConnectionBw 5" << std::endl;
	// start some flows
	std::vector<uint32_t> candidateList = Macroflow::GetCandidateList(this, m_prioritized);
	uint32_t toStart = std::min(m_maxConnection - m_activeHosts.size(), candidateList.size()); // must >= 0
	//uint64_t now = Simulator::Now().GetMilliSeconds();
	//if(now - m_lastClock >= 2000){
	//if(now  >= 30*1000){
	//	std::cout<<"to start: "<<toStart<<std::endl;
	//}


	std::vector<uint32_t> chosenHost;
	while(toStart--){
		std::vector<uint32_t>::iterator chosenIt;
		uint32_t chosenMachinePos = -1; // largest flow

		// modified by lkx... according to the paper
		// Z 项目中是false
		// so i haven't modify it 
		if(GlobalProperty::m_adjust_priority){
			if(m_prioritized){
				for(std::vector<uint32_t>::iterator it = candidateList.begin();it!=candidateList.end();++it){// the index smaller, the link larger
					uint32_t machinePos = -1;
					for(uint32_t i = 0;i<m_mapperHostList.size();++i){
						if(m_mapperHostList[i].m_machineId==*it){ machinePos = i; break; }
					}
					// Z 为啥会有size是0的呢
					if(m_mapperHostList[machinePos].m_size==0) {continue;}
					chosenMachinePos = machinePos; chosenIt = it;
					break; // get the max bw
				}
				// Z 这里也是pending，看来上面可能没啥问题
				if(m_mapperHostList[chosenMachinePos].m_state != AggregatedMapper::Pending) {
					NS_FATAL_ERROR("flow has started!!");
				}
			} else{
				for(std::vector<uint32_t>::iterator it = candidateList.begin();it!=candidateList.end();++it){
					uint32_t machinePos = -1;
					for(uint32_t i = 0;i<m_mapperHostList.size();++i){
						if(m_mapperHostList[i].m_machineId==*it){ machinePos = i; break; }
					}
					if(m_mapperHostList[machinePos].m_size==0) {continue;}
					// Z 这里的第一个条件也是为了让第一次可以进入判断 
					if(chosenMachinePos == (uint32_t)-1 ||
							m_mapperHostList[chosenMachinePos].m_size > m_mapperHostList[machinePos].m_size){
						chosenMachinePos = machinePos; chosenIt = it;
					}
					break; // get the max bw
				}
				if(m_mapperHostList[chosenMachinePos].m_state != AggregatedMapper::Pending) {
					NS_FATAL_ERROR("flow has started!!");
				}
			}
		}else{
			for(std::vector<uint32_t>::iterator it = candidateList.begin();it!=candidateList.end();++it){
				uint32_t machinePos = -1;
				for(uint32_t i = 0;i<m_mapperHostList.size();++i){
					if(m_mapperHostList[i].m_machineId==*it){ machinePos = i; break; }
				}
				if(m_mapperHostList[machinePos].m_size==0) {continue;}
				if(chosenMachinePos == (uint32_t)-1 ||
						m_mapperHostList[chosenMachinePos].m_size < m_mapperHostList[machinePos].m_size){
					chosenMachinePos = machinePos; chosenIt = it;
				}
				break; // get the min remaining size
			}
			if(m_mapperHostList[chosenMachinePos].m_state != AggregatedMapper::Pending) {
				NS_FATAL_ERROR("flow has started!!");
			}
		}


		candidateList.erase(chosenIt);
		chosenHost.push_back(m_mapperHostList[chosenMachinePos].m_machineId);
		m_mapperHostList[chosenMachinePos].m_state = AggregatedMapper::Started;
		m_activeHosts.insert(m_mapperHostList[chosenMachinePos].m_machineId); // add the newly-started flow to active list
		/*
		// Z 使用NS3api启动流
		Ptr<Node> from = GlobalProperty::m_nodeList[m_mapperHostList[chosenMachinePos].m_machineId];
		Ipv4Address to = GlobalProperty::m_ipList[this->GetReducerMachine()];
		Time reducerDelay = MicroSeconds(rand() % GlobalProperty::m_reducerMaxDelayUs);
		// Z how to create a flow
		Ptr<BulkSendApplication> flow = TcpFlowHelper::CreatePendingTcpFlow(from, to, m_mapperHostList[chosenMachinePos].m_size, this->GetDscp(), reducerDelay); // create the flow
		flow->SetMapperHostNumber(m_mapperHostList[chosenMachinePos].m_machineId);// write the mapper machine information to flow structure
		flow->SetParent((void*)this);
		flow->Continue(); // flow is pending when created, thus we need to start it here
		m_mapperHostList[chosenMachinePos].m_flow = flow; // update the flow information
		*/
		Ptr<BulkSendApplication> flow = m_mapperHostList[chosenMachinePos].m_flow;
		flow->AddPrioritizedMf(m_macroflowId);
		flow->Continue();
	}
	if(!chosenHost.empty()) Macroflow::ReportChosenHosts(this,chosenHost);
}


void
Macroflow::RemoveFinishedFlows()
{
	// note that not all finished flows are removed by this function!
	// when a flow is found finished by adjustConnections, it can also remove it directly
	bool finished = false;
	int count = 0;//added by lkx
	// Z 遍历本reducer的流，发现有结束的，就把它从m_activeHosts中去掉
	for(std::vector<AggregatedMapper>::iterator it = m_mapperHostList.begin();it!=m_mapperHostList.end();){
		// Z 这里为啥只有一个流？这个抽象论文里没有提到阿。。。。
		// Z 好像没啥问题
		Ptr<BulkSendApplication> flow = it->m_flow;
		// if(it->m_state == AggregatedMapper::Pending) continue; // flow is NULL!

		// if(flow->IsMfFinished(m_macroflowId) && it->m_state!=AggregatedMapper::Finished){ // use the 2nd condition to guarantee that each flow is calculated by only 1 time.
		if(flow->IsMfFinished(m_macroflowId)){
			++ m_finishedFlows;

			std::set<uint32_t>::iterator its = m_activeHosts.find(it->m_machineId);
			// Z 如果找到的话就把这个去掉
			if(its!=m_activeHosts.end())m_activeHosts.erase(its);
			// else NS_FATAL_ERROR("cannot erase a flow from active list");
			finished = true;
			
			m_mapperHostList.erase(it);
		}
			
		else{
			it++;
			//if(this->GetReducerMachine()==0)std::cout<<&(*it)<<" "<<it->m_machineId<<" error"<<std::endl;
			//std::cout<<it->m_state<<std::endl;
			// NS_FATAL_ERROR("cannot find an active-state macroflow on a given host");
		}
		
	}
	if(finished){
		if(GlobalProperty::m_bwAlgorithm){
			//if(m_priority)adjustConnectionBw();
			//else --m_maxConnection;
			//modified by lkx...
			//if(m_priority) {std::cout<<"p "<<m_maxConnection<<" "<<count<<std::endl; adjustConnectionBw();}
			//else {m_maxConnection -= count; }
			// Z 如果m_priority是0，则在m_maxConnection减去结束的流的数量
			// Z TODO 不是很懂这个是干嘛
			if(!m_priority) m_maxConnection-=count;
			adjustConnectionBw();

		}
		//else adjustConnection();
	}
}



} /* namespace ns3 */
