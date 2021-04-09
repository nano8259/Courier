/*
 * Tracker.cc
 *
 *  Created on: Oct 7, 2017
 *      Author: tbc
 */

#include "tracker.h"

namespace ns3 {

bool Tracker::initialized = false;
std::vector<double> Tracker::uLink;
std::vector<double> Tracker::dLink;
std::vector<uint32_t> Tracker::uConn;
std::vector<Ptr<ParallelJob> > Tracker::uPrio;

std::map<uint32_t,std::vector<uint32_t> > Tracker::senderList;
SenderInfo Tracker::sender[600][160];// i-th coflow, j-th mapper machine
SenderInfo Tracker::receiver[600][160];// i-th coflow, j-th mapper machine


// called by Tracker::UpdateBandwidth()
void Tracker::init(){
	initialized = true;
	// Z 对4个vector进行初始化
	for(uint32_t i=0;i<GlobalProperty::m_noMachines;++i){
		uLink.push_back(0); dLink.push_back(0); uConn.push_back(0); uPrio.push_back(Ptr<ParallelJob>());
	}
}

// called by MasterApplication::DoSchedule(). must be called after reducer-machine of reducer has been set
// Z 在senderList这个map中加入pair对应的flows的机器id（也就是mapperlist？）
void Tracker::ReportMapperList(Ptr<Macroflow> reducer){
	ParallelJob* job = (ParallelJob*)(reducer->GetJob().operator ->());
	// uint32_t pair = makepair(job->GetId(),reducer->GetId());
	uint32_t pair = reducer->GetId();
	std::map<uint32_t,std::vector<uint32_t> >::iterator it = senderList.find(pair);
	if(it!=senderList.end())
		NS_FATAL_ERROR("mapper-list is reported by twice");
	std::vector<uint32_t> list;
	const std::vector<AggregatedMapper>& flows = reducer->GetFlows();
	for(uint32_t i=0;i<flows.size();++i){
		if(GlobalProperty::m_bwAlgorithm){
			if(flows[i].m_machineId == reducer->GetReducerMachine())
				continue; // skip local flows
		}
		list.push_back(flows[i].m_machineId);
	}
	senderList[pair] = list;
}


// called by MasterApplication::DoSchedule()
// update uplink and downlink usage
void Tracker::UpdateBandwidth(const std::vector<Ptr<Macroflow> >& mfs){

	if(!initialized) init();
	// Z 初始化数组，按传入的活跃流的数据算带宽
	for(uint32_t i=0;i<GlobalProperty::m_noMachines;++i){
		uLink[i] = 0; dLink[i]=0; uConn[i]=0;
	}
	uint64_t tm = Simulator::Now().GetMilliSeconds();
	// Z 对传入的vector的每个mf进行迭代
	for(uint32_t i=0;i<mfs.size();++i){
		Ptr<Macroflow> mf = mfs[i];
		const std::vector<AggregatedMapper>& flows = mf->GetFlows();
		Ptr<ParallelJob> job = (ParallelJob*) mf->GetJob().operator ->();
		uint32_t jobId = job->GetId();
		// Z 这里的flows是AggregatedMapper的vector
		// Z 而flow的.m_flow是BulkSendApplication。。。
		for(uint32_t j=0;j<flows.size();++j){
			if(flows[j].m_state != AggregatedMapper::Started)
				continue; // because flows[j].m_flow is NULL
			Ptr<BulkSendApplication> flow = flows[j].m_flow;
			uint32_t src = flow->GetMapperHostNumber();
			uint32_t dst = mf->GetReducerMachine();
			if(src==dst) // skip local flows
				continue;
			// Z TODO 这个speed是怎么计算的？
			double speed = flow->GetMfCurrentSpeed(mf->GetId()); // we do not call estimateXXX here, for it's called by master-application
			speed /= 1024*(1024/8); // to Mbps
			uLink[src] += speed; dLink[dst] += speed; ++uConn[src];
			// Z 因为是在同一个tm下的多次循环，所以在操作前验证之前这个量检测了没有
			if(sender[jobId][src].timeStamp == tm) sender[jobId][src].bw+=speed;
			else sender[jobId][src].bw=speed, sender[jobId][src].timeStamp = tm;
			if(receiver[jobId][dst].timeStamp == tm) receiver[jobId][dst].bw+=speed;
			else receiver[jobId][dst].bw=speed, receiver[jobId][dst].timeStamp = tm;
		}
		for(uint32_t j=0;j<flows.size();++j){
			if(flows[j].m_size==0) continue;
			uint32_t src = flows[j].m_machineId;
			// Z 这里是和Django的算法有关的内容，计算优先级，某机器上优先级最高的流是剩余发送量最小的流
			// Z 用于接下来的GetCandidateList
			if(!uPrio[src] || uPrio[src]->GetTotalBytes()-uPrio[src]->GetSentBytes() > job->GetTotalBytes()-job->GetSentBytes() )
				uPrio[src] = job;
		}

	}
	double mx = 1e9/1024.0/1024.0;
	// Z 剩余上传链路容量是上传总容量和占用容量的较小值（有可能占用容量大于上传容量？
	for(uint32_t i=0;i<GlobalProperty::m_noMachines;++i){
		uLink[i] = std::min(uLink[i],mx); dLink[i] = std::min(dLink[i],mx);
	}
	/*std::cout<<"Time: "<<Simulator::Now().GetMilliSeconds()<<std::endl;
	for(uint32_t i=0;i<GlobalProperty::m_noMachines;++i)
		std::cout<<(uint32_t)uLink[i]<<"  ";
	std::cout<<std::endl;
	for(uint32_t i=0;i<GlobalProperty::m_noMachines;++i)
		std::cout<<(uint32_t)dLink[i]<<"  ";
	std::cout<<std::endl;
	for(uint32_t i=0;i<GlobalProperty::m_noMachines;++i)
		std::cout<<uConn[i]<<"    ";
	std::cout<<std::endl;*/
}

int ulinkCmp(const uint32_t& a, const uint32_t& b){
	return Tracker::uLink[a] < Tracker::uLink[b];
}
int uconnCmp(const uint32_t& a, const uint32_t& b){
	return Tracker::uConn[a] < Tracker::uConn[b];
}

// called by Macroflow::adjustConnection()
// Z 对给定的reducer，选择几个合适的mapper的机器给他
// 优先的话，就是说对job中几个特定的机器，减少他们的占用带宽和流量数
std::vector<uint32_t> Tracker::GetCandidateList(Ptr<Macroflow> reducer, bool prioritized) {
	if(!initialized) init();
	ParallelJob* job = (ParallelJob*)(reducer->GetJob().operator ->());
	// Z 对两个vector的备份
	std::vector<double> uLinkBak = uLink;
	std::vector<uint32_t> uConnBak = uConn;
	if(GlobalProperty::m_bwAlgorithm && prioritized){
		for(uint32_t i=0;i<uPrio.size();++i){
			// Z 如果该机器的优先job和reducer的job是同一个的话
			if(!!uPrio[i] && uPrio[i]->GetId()==job->GetId()) {
				// Z TODO 这个m_senderPromotion是啥，为啥跟流量/数量都可以进行比较？
				// Z 师姐给的代码中，m_senderPromotion为200
				uLink[i] = uLink[i] - (double)GlobalProperty::m_senderPromotion; // allow negative
				// uConn -= m_senderPromotion，下限为0
				uConn[i] = (uConn[i]>GlobalProperty::m_senderPromotion)?(uConn[i]-GlobalProperty::m_senderPromotion):0;
			}
		}
	}

	std::vector<uint32_t> result;
	// Z TODO 这俩do while false是啥意思阿
	do{
		// uint32_t pair = makepair(job->GetId(),reducer->GetId());
		uint32_t pair = reducer->GetId();
		// Z senderlist：<pair,对应的flows的机器i的列表>
		std::map<uint32_t,std::vector<uint32_t> >::iterator it = senderList.find(pair);
		if(it==senderList.end()) break;
		std::vector<uint32_t>& list = it->second;
		// Z 项目中默认是true
		if(GlobalProperty::m_bwAlgorithm == false){
			// Z TODO n是啥  job的mapper数量的一半和5的最大值 和reducer的机器列表的大小的最小值
			uint32_t n = std::max(job->GetNoMappers() / 2, 5u);
			n = std::min(n, (uint32_t)list.size());
			uint32_t* randomIndex = new uint32_t[list.size()];
			for(uint32_t i=0;i<list.size();++i) randomIndex[i] = i;
			std::random_shuffle(randomIndex,randomIndex+list.size());
			// 返回list的n个随机取值？
			for(uint32_t i=0;i<n;++i) result.push_back(list[randomIndex[i]]);
			delete[] randomIndex;
		}else{
			//double coflowBw = receiver[job->GetId()][reducer->GetReducerMachine()].bw;
			// Z 总带宽是reducer机器的被占用下载带宽
			double totalBw = dLink[reducer->GetReducerMachine()];
			// Z 对应的机器id列表进行shuffle
			std::random_shuffle(it->second.begin(),it->second.end());

			if(prioritized){
				double percent[4] = {0.25,0.5,0.75,1.0};
				// Z GetDigit：第一个参数除以第二个参数模10，大概就是整个随机数把
				double maxsize = (uint32_t)(GlobalProperty::m_noMachines*percent[GetDigit(GlobalProperty::m_bwAlgorithmDetails,1)]);
				uint32_t n = std::min((uint32_t)maxsize,(uint32_t)it->second.size());
				// Z 感觉不是随机数，而是每一位代表一定含义，项目默认为90000211
				// Z 排序方式不同，一个是按uconn，一个是按ulink
				if(GetDigit(GlobalProperty::m_bwAlgorithmDetails,10)==0) std::sort(it->second.begin(),it->second.end(),uconnCmp);
				else std::sort(it->second.begin(),it->second.end(),ulinkCmp);
				for(uint32_t i=0;i<n;++i) result.push_back(it->second[i]);
			}else{
				// Z TODO 这俩do while false是啥意思阿
				do{
					// Z 项目默认是900
					double speed_limit_tracker = GlobalProperty::m_speed_limit_tracker;
					// Z 如果现在的被占用的带宽比设定的大，就break
					if(totalBw>speed_limit_tracker) break;
					if(GetDigit(GlobalProperty::m_bwAlgorithmDetails,100)==0){ // connections
						// Z 按连接数升序排序，给出几个最小的
						std::sort(it->second.begin(),it->second.end(),uconnCmp);
						uint32_t n = std::min(3u,(uint32_t)it->second.size());
						for(uint32_t i=0;i<n;++i) result.push_back(it->second[i]);
					}else{
						std::sort(it->second.begin(),it->second.end(),ulinkCmp); // bw
						if(GetDigit(GlobalProperty::m_bwAlgorithmDetails,100)==1){ // max bw
							// Z 按已占用的带宽排序，给出几个最小的
							uint32_t n = std::min(3u,(uint32_t)it->second.size());
							for(uint32_t i=0;i<n;++i) result.push_back(it->second[i]);
						}else{ // best-fitted bw
							//double remBw = std::max(900-totalBw,0.0);
							// Z 剩下的带宽就是设定的限制-占用的，不能为负
							double remBw = std::max(speed_limit_tracker-totalBw,0.0);
							uint32_t chosen = 0; double temp = 1e10;
							// Z 选一个剩余带宽和当前的上传带宽最接近的
							for(uint32_t i=0;i<it->second.size();++i){
								double uremBw = std::max(900-uLink[it->second[i]],0.0);
								if(fabs(uremBw-remBw) < fabs(temp-remBw)){ temp = uremBw; chosen = i; }
							}
							//...
							while(chosen>0 && uLink[it->second[chosen]]>850) --chosen;
							// 大约选三个，最接近的
							for(uint32_t i = std::max(chosen-1,0u);i<=std::min((uint32_t)it->second.size()-1,chosen+1);++i)
								result.push_back(it->second[i]);
						}
					}
				}while(false);
			}
		}
	}while(false);
	// Z 恢复两个备份（为啥不修改复制的那一份呢？）
	uLink = uLinkBak;
	uConn = uConnBak;
	return result;
}

// called by Macroflow::adjustConnection()
// 把给的reducer的senderlist中被选中的删除
void Tracker::ReportChosenHosts(Ptr<Macroflow> reducer, const std::vector<uint32_t>& chosenSender) {
	ParallelJob* job = (ParallelJob*)(reducer->GetJob().operator ->());
	// uint32_t pair = makepair(job->GetId(),reducer->GetId());
	uint32_t pair = reducer->GetId();
	std::map<uint32_t,std::vector<uint32_t> >::iterator it = senderList.find(pair);
	if(it==senderList.end())
		NS_FATAL_ERROR("Bug: no such reducer!");

	uint32_t n = GlobalProperty::m_noMachines; // each machine has at most 1 aggregated mapper
	bool* isChosen = new bool[n];
	for(uint32_t i=0;i<n;++i) isChosen[i] = false;
	for(uint32_t i=0;i<chosenSender.size();++i) isChosen[chosenSender[i]] = true;
	for(std::vector<uint32_t>::iterator itv = it->second.begin();itv!=it->second.end();){
		// Z erase返回itv的下一个
		if(isChosen[*itv]) itv = it->second.erase(itv);
		else ++itv;
	}
	delete[] isChosen;
}

// Z 传入的第一个参数是reducer，第二个参数是被停止的host的列表
void Tracker::ReportPausedHosts(Ptr<Macroflow> reducer, const std::vector<uint32_t>& pausedSender) {
	ParallelJob* job = (ParallelJob*)(reducer->GetJob().operator ->());
	// uint32_t pair = makepair(job->GetId(),reducer->GetId());
	uint32_t pair = reducer->GetId();
	std::map<uint32_t,std::vector<uint32_t> >::iterator it = senderList.find(pair);
	if(it==senderList.end())
		NS_FATAL_ERROR("Bug: no such reducer!");
	// Z 所以这个senderlist是可用的sender的list
	for(uint32_t i=0;i<pausedSender.size();++i){
		if(std::find(it->second.begin(),it->second.end(),pausedSender[i]) != it->second.end())
			NS_FATAL_ERROR("sender host has already in list");
		it->second.push_back(pausedSender[i]);
	}
}

} /* namespace ns3 */
