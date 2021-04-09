/*
 * machine-slots.cc
 *
 *  Created on: Jun 9, 2017
 *      Author: tbc
 */

// Z 这个类好像就是4.2Local Scheduler

#include "machine-slots.h"

namespace ns3 {

MachineSlots::MachineSlots() { }

MachineSlots::MachineSlots(uint32_t machineId) {
	m_machineId = machineId;
	// init the map of id and app(flow)s
	for(uint32_t i = 0; i < GlobalProperty::m_noMachines; i++){
		if(i != machineId){
			Ptr<Node> from = GlobalProperty::m_nodeList[i];
			Ipv4Address to = GlobalProperty::m_ipList[machineId];
			Time reducerDelay = MicroSeconds(rand() % GlobalProperty::m_reducerMaxDelayUs);
			// from to size DscpDefault 
			// ZTODO the problem is the delay is fixed
			Ptr<BulkSendApplication> flow = TcpFlowHelper::CreatePendingTcpFlow(from, to, 0, 0, reducerDelay); // create the flow
			flow->SetMapperHostNumber(i);// write the mapper machine information to flow structure
			flow->SetParent((void*)this);
			m_flows.insert(std::make_pair(i, flow));
		}
	}
}

MachineSlots::~MachineSlots() { }


void MachineSlots::AddMf(Ptr<Macroflow> mf){
	m_list.push_back(mf);
	// check if flows is finish
	std::map<uint32_t, Ptr<BulkSendApplication>>::iterator it;
	for(it = m_flows.begin(); it != m_flows.end(); it++){
		if(it->second->IsFinished()){
			// if the flow is not connected, means it is finished
			// create a new flow
			Ptr<Node> from = GlobalProperty::m_nodeList[it->first];
			Ipv4Address to = GlobalProperty::m_ipList[m_machineId];
			Time reducerDelay = MicroSeconds(rand() % GlobalProperty::m_reducerMaxDelayUs);
			// from to size DscpDefault 
			// ZTODO the problem is the delay is fixed
			Ptr<BulkSendApplication> flow = TcpFlowHelper::CreatePendingTcpFlow(from, to, 0, 0, reducerDelay); // create the flow
			flow->SetMapperHostNumber(it->first);// write the mapper machine information to flow structure
			flow->SetParent((void*)this);
			// m_flows.insert(std::make_pair(i, flow));
			it->second = flow;
		}
	}
	mf->SetFlows(m_flows);
}


void MachineSlots::RemoveMf(Ptr<Macroflow> mf){
	for(std::vector<Ptr<Macroflow> >::iterator it = m_list.begin();it!=m_list.end();++it){
		// 这里为啥要使用->()运算符？
		if((*it).operator ->() == mf.operator ->()){
			//m_szFinishedMfs += (*it)->GetSentBytes();
			m_list.erase(it);
			return;
		}
	}
	NS_FATAL_ERROR("MachineSlots::RemoveMF - no such macroflow");
}

// Z 比较剩下没有发送的Byte数，如果两个mf是不同job，比较job剩下的，如果是同一个job，比较mf剩下的
int sizeComp(const Ptr<Macroflow>& a, const Ptr<Macroflow>& b){
	ParallelJob* ja = (ParallelJob*) a->GetJob().operator ->();
	ParallelJob* jb = (ParallelJob*) b->GetJob().operator ->();
	if(ja->GetId() != jb->GetId())
		return ja->GetTotalBytes()-ja->GetSentBytes() < jb->GetTotalBytes()-jb->GetSentBytes();
	else
		return a->GetBytes()-a->GetSentBytes() < b->GetBytes()-b->GetSentBytes();
}

// called by MasterApplication::DoSchedule()
void MachineSlots::ReallocateFetchers() {
	// Z !!!!我人晕了????每个机器上的fetcher数量是写死的，跟论文上的完全不一样
	// Z 论文上的意思应该是会根据丢包率动态的更改
	// Z 修改成1试一下
	uint32_t total = GlobalProperty::m_maxFetchers; // max fetchers per machine
	//uint32_t first = (uint32_t)(total*0.7);
	// m_connection_limit = 0.7 但是不知道这个值是什么意思
	// Z 加0.5，四舍五入
	uint32_t first = (uint32_t)(total*GlobalProperty::m_connection_limit + 0.5);
	// std::cout << "<MachineSlot> before m_bwAlgorithm" << std::endl;
	if(GlobalProperty::m_bwAlgorithm == false){
		// currently we give all fetchers to the coflow that has the highest priority
		// Z 最高优先级也就是最小的剩余Byte数
		Ptr<Macroflow> chosen;
		uint64_t chosenBytes = static_cast<uint64_t>(-1);
		for(uint32_t i=0;i<m_list.size();++i){
			Ptr<Macroflow> mf = m_list[i];
			if(mf->GetState() == Macroflow::Finished) continue; // excluding finished reducer
			ParallelJob* job = (ParallelJob*)(mf->GetJob().operator ->());
			uint64_t remainingBytes = job->GetTotalBytes() - job->GetSentBytes();
			// Z 这里怎么好像只改了mf，没改chosenBytes
			if(remainingBytes<chosenBytes) chosen = mf;
		}
		for(uint32_t i=0;i<m_list.size();++i){
			// Z 把所有的fetcher全部给chosen的那个，其他的连接数都设置成0
			if(m_list[i].operator ->() == chosen.operator ->()) m_list[i]->SetMaxConnection(total);
			else m_list[i]->SetMaxConnection(0);
		}
	}else{
		// Z bw算法
		// Z m_list是本机器上macroflow的数量
		if(m_list.empty()) return;
		// std::sort(m_list.begin(),m_list.end(),sizeComp);//first sort by left coflow size, then sort by left macroflow size
		uint32_t usedConn = 0;
		for(uint32_t i=0;i<m_list.size();++i)
			usedConn +=m_list[i]->GetMaxConnection();

		/*if(Simulator::Now().GetMilliSeconds()>30*1000){
			std::cout<<getSpeed()<<std::endl;
			for(uint32_t i=0;i<m_list.size();++i)
				std::cout<<m_list[i]->GetMaxConnection()<<" ";
			std::cout<<std::endl;
			exit(0);
		}*/
		// std::cout << "<MachineSlot> in else 1" << std::endl;
		
		//algorithm 1...
		//if the priority coflow is not changed
		if(m_list[0]->GetMaxConnection() == first){ // keep
		// std::cout << "<MachineSlot> in else 2" << std::endl;
			// Z speed就是目前占用的带宽转化为Mbps
			double speed = getSpeed();
			//if(speed>920) return;
			// Z 现在的速度很高,可能说明带宽已满?
			if(speed > GlobalProperty::m_speed_limit) return;
			// std::cout << "<MachineSlot> in else 3" << std::endl;
			for(uint32_t i=1;i<m_list.size();++i){
				uint32_t remain = total-usedConn;
				if(total<=usedConn) {
					for(uint32_t i=0;i<m_list.size();++i)
						// Z 这句是在开什么玩笑。。。是为了通知更新吗
						m_list[i]->SetMaxConnection(m_list[i]->GetMaxConnection());
					/*if(Simulator::Now().GetMilliSeconds()>30*1000){
						std::cout<<"total "<<total<<"usedConn "<<usedConn<<std::endl;
						exit(0);
					}*/
					// 如果用的>=总计的，那么每个reducer的流更新一遍，然后返回。。
					return;
				}
				uint32_t currentConn = m_list[i]->GetMaxConnection();
				if(currentConn >= remain/2) continue;
				// Z 如果该reducer使用的流的数量小于remain的一半，则为该reducer+1，优先设置为false
				m_list[i]->SetPriority(false);
				m_list[i]->SetMaxConnection(currentConn+1);
				// Z 设置一个流+1后就return
				return;
			}
		}else{// reallocate
			// std::cout << "<MachineSlot> in else 4 list size" << m_list.size() << std::endl;
			m_list[0]->SetPriority(true);
			m_list[0]->SetMaxConnection(first);
			// std::cout << "<MachineSlot> in else 4.1" << std::endl;
			for(uint32_t i=1;i<m_list.size();++i){
				m_list[i]->SetPriority(false);
				// std::cout << "<MachineSlot> in else 4.2" << std::endl;
				// 为上一个优先的reducer的流的数量设置为total-frist(0.3total)
				if(m_list[i]->GetMaxConnection()>=first)
					m_list[i]->SetMaxConnection(total-first);
					// std::cout << "<MachineSlot> in else 4.3" << std::endl;
			}
		}
		// std::cout << "<MachineSlot> in else 5" << std::endl;
		// 通知一遍其他reducer
		for(uint32_t i=0;i<m_list.size();++i)
			m_list[i]->SetMaxConnection(m_list[i]->GetMaxConnection());
	}
	// std::cout << "<MachineSlot> end reallocate" << std::endl;
}

double MachineSlots::getSpeed() const {
	double bw = 0;
	for(uint32_t i=0;i<m_list.size();++i){
		const std::vector<AggregatedMapper> flows = m_list[i]->GetFlows();
		for(uint32_t j = 0;j<flows.size();++j){
			if(flows[j].m_state == AggregatedMapper::Started)
				bw += flows[j].m_flow->GetCurrentSpeed();
		}
	}
	return bw / (1024/8.0*1024); //Mbps
}


} /* namespace ns3 */
