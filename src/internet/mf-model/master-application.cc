/*
 * master-application.cc
 *
 *  Created on: Mar 17, 2017
 *      Author: tbc
 */

#include "master-application.h"

namespace ns3 {

MasterApplication::MasterApplication() {
	Log0("start master application");
	m_isStop = false;

	std::string mfsType = GlobalProperty::m_mfSchedulerType;
	std::string mfsSubtype = GlobalProperty::m_mfSchedulerSubtype;
	ToLower(mfsType);
	ToLower(mfsSubtype);
	// Z 这里好像三个调度器都初始化了？
	if(mfsType == "smf"){
		if(mfsSubtype == "exp")
			m_mfs = new MfSchedulerSmf(MfSchedulerSmf::Exp);
		else
			NS_FATAL_ERROR ("Unknown mf-scheduler-subtype: smf-"+mfsSubtype);
	}else{
		NS_FATAL_ERROR ("Unknown mf-scheduler-type: "+mfsType);
	}

	std::string nisType = GlobalProperty::m_niSchedulerType;
	ToLower(nisType);
	if(nisType == "fair")
		m_nis = new NiScheduler(NiScheduler::Fair);
	else if(nisType=="size")
		m_nis = new NiScheduler(NiScheduler::Size);
	else
		NS_FATAL_ERROR ("Unknown ni-scheduler-type: "+nisType);

	std::string cisType = GlobalProperty::m_ciSchedulerType;
	ToLower(cisType);
	if(cisType == "oppo")
		m_cis = new CiScheduler(CiScheduler::Oppo);
	else if(cisType == "peri")
		m_cis = new CiScheduler(CiScheduler::Peri);
	else if(cisType == "none")
		m_cis = new CiScheduler(CiScheduler::None);
	else
		NS_FATAL_ERROR ("Unknown ci-scheduler-type: "+cisType);

	// Z 每个机器上空闲的slot数量
	m_noFreeSlots = std::vector<uint32_t>(GlobalProperty::m_noMachines, GlobalProperty::m_noSlots);
	m_lastClock = 0;
	m_lastMfStart = Seconds(0);
	// Z slots对象的队列
	for(uint32_t i=0;i<GlobalProperty::m_noMachines;++i)
		m_slots.push_back(MachineSlots(i));
	m_speedEstimatorCounter = 0;

	m_totalBytes = 0;
}

MasterApplication::~MasterApplication() {
	delete m_mfs;
	delete m_nis;
	delete m_cis;
}

void
MasterApplication::LoadTraceFile(std::string traceFile)
{
	// Log0("start load trace file");
	TraceReader tr(traceFile);
	m_jobs = tr.Analysis();
	// Z 为8个队列，8个优先级分别计算阈值（指数算法，阈值为总B数**（1/优先级））
	m_mfs->CalcThresholds(m_jobs);
	m_totalBytes = tr.GetTotalBytes();
	// Log0("here loading");
	// prevent from careless configuration
	if(GlobalProperty::m_pcapOutput){
		// Z 大于2GB
		if(m_totalBytes/1024/1024/1024>2)
			NS_FATAL_ERROR("Cannot save pcap-file: traffic too large");
	}
	// Log0("finish load trace file");
}

void
MasterApplication::StartApplication ()
{
	// Log0("start application");
	// a!=b || a==b==0 || no_trace_file
	if(GlobalProperty::m_nodeList.size()!=GlobalProperty::m_ipList.size() || GlobalProperty::m_nodeList.empty())
		NS_FATAL_ERROR ("No node-list or ip-list!");

	// install tcp-sink application to each client node
	// Z 没看见这个sink的输出阿
	for(uint32_t i=0;i<GlobalProperty::m_nodeList.size();++i)
		TcpFlowHelper::InstallSinkApp(GlobalProperty::m_nodeList[i]);

	LoadTraceFile(GlobalProperty::m_traceFile);

	Simulator::ScheduleNow(&MasterApplication::DoSchedule,this);
}

void
MasterApplication::StopApplication ()
{
	m_isStop = true;
}

void
MasterApplication::DoSchedule ()
{
	if(m_isStop)
		return;

	// Log1("start do schedule at %u",(unsigned)Simulator::Now().GetMilliSeconds());

	// Z 只有ni
	std::cout << "<MasterApplication> before 1	at" << (unsigned)Simulator::Now().GetMilliSeconds() << std::endl;
	// [1] update active-job list
	m_nis->CheckJobArrival(m_jobs);
	m_cis->CheckJobArrival();

	// std::cout << "<MasterApplication> before 2	at" << (unsigned)Simulator::Now().GetMilliSeconds() << std::endl;
	// [2] release slot resources
	// [2.1] first release finished slots(ni & ci)
	std::vector<Ptr<Macroflow> > finishedMfs = m_nis->GetJustFinishedMfs();
	for(uint32_t i=0; i<finishedMfs.size(); ++i){
		uint32_t reducerMachine = finishedMfs[i]->GetReducerMachine();
		++m_noFreeSlots[reducerMachine];
		m_slots[reducerMachine].RemoveMf(finishedMfs[i]);
		// Log1("free slot after release a reducer task: %u",SumVec(m_noFreeSlots));
	}

	// [2.2] release slots occipied by finished ci-jobs
	// for ci-jobs, we just record their slots, instead of emulate them
	std::vector<Ptr<ParallelJob> > finishedCiJobs = m_cis->GetJustFinishedJobs();
	for(uint32_t i=0;i<finishedCiJobs.size();++i){
		// note that we modify the inner-properties of a job in ci-scheduler
		// so we just update the slot states here
		++m_noFreeSlots[finishedCiJobs[i]->GetCiMachine()];
	}

	// std::cout << "<MasterApplication> before 3.*	at" << (unsigned)Simulator::Now().GetMilliSeconds() << std::endl;
	//[3.* estimate flow speed]
	const std::vector<Ptr<Macroflow> >& activeMfsBeforeScheduling = m_nis->GetActiveMfList();
	// Z 配置中这个周期是1
	m_speedEstimatorCounter = (m_speedEstimatorCounter+1) % GlobalProperty::m_speedEstimatorPeriod;
	if(m_speedEstimatorCounter == 0){
		// for(uint32_t i=0;i<activeMfsBeforeScheduling.size(); ++i){
		// 	const std::vector<AggregatedMapper>& flows = activeMfsBeforeScheduling[i]->GetFlows();
		// 	for(uint32_t j=0;j<flows.size();++j){
		// 		if(flows[j].m_state == AggregatedMapper::Pending) continue; // flows[j].m_flow is NULL
		// 		if(flows[j].m_flow->IsMfFinished(activeMfsBeforeScheduling[i]->GetId())) continue;
		// 		// not necessary
		// 		TcpFlowHelper::EstimateSpeed(flows[j].m_flow);
		// 	}
		// }
		for(uint32_t i = 0; i < m_slots.size(); i++){
			std::map <uint32_t, ns3::Ptr<ns3::BulkSendApplication>> ::iterator it;
			for(it = m_slots[i].m_flows.begin(); it != m_slots[i].m_flows.end(); it++){
				TcpFlowHelper::EstimateSpeed(it->second);
			}
		}
		// 调用tracker更新带宽
		Tracker::UpdateBandwidth(activeMfsBeforeScheduling);
	}

	// std::cout << "<MasterApplication> before 3.1	at" << (unsigned)Simulator::Now().GetMilliSeconds() << std::endl;
	// [3] scheduling jobs
	// [3.1] scheduling ni-jobs
	while(SumVec(m_noFreeSlots)>0 && m_nis->HasWaitingJob()){
		// find a slot and a macroflow
		Ptr<Macroflow> mf = m_nis->GetReducerTask();

		uint32_t reducerMachine = 0;
		// Z true
		if(GlobalProperty::m_uniformSlotAllocation)
			reducerMachine = MaxPos(m_noFreeSlots);
		else
			reducerMachine = FirstPositive(m_noFreeSlots);
		if(GlobalProperty::m_log_state) Log1("Reducer Machine: %d",reducerMachine); //lkx

		mf->SetReducerMachine(reducerMachine);
		Tracker::ReportMapperList(mf); // must be called after setting reducer machine
		
		// Z 设置中一个机器4个slot
		--m_noFreeSlots[reducerMachine];
		m_slots[reducerMachine].AddMf(mf);
		// Z 记录最后一个mf的启动
		m_lastMfStart = Simulator::Now();
		if(GlobalProperty::m_log_state) Log1("free slot after emit a reducer task: %u",SumVec(m_noFreeSlots));
	}
	//each item in m_slots stands for a machine...
	for(uint32_t i=0;i<m_slots.size();++i){
		// Z 通知每一个机器reallocate
		// std::cout << "<MasterApplication> reallocate" << std::endl;
		m_slots[i].ReallocateFetchers();
	}

	// std::cout << "<MasterApplication> before 3.2	at" << (unsigned)Simulator::Now().GetMilliSeconds() << std::endl;
	// [3.2] scheduling ci-job
	// for ci-jobs, we just record their slots, instead of emulate them
	while(SumVec(m_noFreeSlots)>0 && m_cis->HasWaitingJob()){
		Ptr<ParallelJob> job = m_cis->GetComputationTask();
		uint32_t computationMachine = 0;
		if(GlobalProperty::m_uniformSlotAllocation)
			computationMachine = MaxPos(m_noFreeSlots);
		else
			computationMachine = FirstPositive(m_noFreeSlots);

		--m_noFreeSlots[computationMachine];
		job->setCiMachine(computationMachine);
	}

	// std::cout << "<MasterApplication> before 3.3	at" << (unsigned)Simulator::Now().GetMilliSeconds() << std::endl;
	// [3.3] update mf priority and estimate current flow speed
	// const std::vector<Ptr<Macroflow> >& activeMfs = m_nis->GetActiveMfList();
	// for(uint32_t i=0;i<activeMfs.size(); ++i){
	// 	// Z 把所有mf的优先级变为7
	// 	m_mfs->SetPriority(activeMfs[i]);
	// 	const std::vector<AggregatedMapper>& flows = activeMfs[i]->GetFlows();
	// 	for(uint32_t j=0;j<flows.size();++j){
	// 		if(flows[j].m_state == AggregatedMapper::Pending) continue; // flows[j].m_flow is NULL
	// 		if(flows[j].m_flow->IsMfFinished(activeMfs[i]->GetId())) continue;
	// 		// Z 这里的话就是把所有的bulkapp的优先级都变成default(0)，然而本来也都是0.。。。。。。
	// 		TcpFlowHelper::UpdateDscp(flows[j].m_flow, activeMfs[i]->GetDscp());
	// 		//std::cout<<"active "<<int(activeMfs[i]->GetDscp())<<std::endl;
	// 	}
	// }

	// std::cout << "<MasterApplication> before 4	at" << (unsigned)Simulator::Now().GetMilliSeconds() << std::endl;
	// [4] stop condition
	if(!m_nis->IsAllFinished() || !m_cis->IsAllFinished()){
		// 循环调用自己
		Simulator::Schedule(GlobalProperty::m_simulationStep,&MasterApplication::DoSchedule,this);
	}else{
		m_isStop = true;
		// stop the flows
		std::cerr << "try to stop all the flows" << std::endl; // for debug
		std::vector<MachineSlots>::iterator itms;
		for(itms = m_slots.begin(); itms != m_slots.end(); itms++){
			std::map<uint32_t, ns3::Ptr<ns3::BulkSendApplication>>::iterator itmp;
			for(itmp = itms -> m_flows.begin(); itmp != itms -> m_flows.end(); itmp++){
				itmp->second->StopWithoutCallback();
			}
		}
		OutputMeasurements();
	}
}

void
MasterApplication::OutputMeasurements() const
{
	std::vector<uint64_t> niJctList;
	std::vector<uint64_t> ciJctList;
	std::vector<uint64_t> cctList;
	std::vector<uint64_t> mctList;
	std::vector<uint64_t> jctList;

	for(uint32_t i = 0;i<m_jobs.size();++i){
		Ptr<ParallelJob> job = m_jobs[i];
		niJctList.push_back( job->GetFinishTime().GetMilliSeconds() - job->GetArrivalTime().GetMilliSeconds() );
		cctList.push_back( job->GetFinishTime().GetMilliSeconds() - job->GetStartTime().GetMilliSeconds() - job->GetCiDuration().GetMilliSeconds() ); // note that we exclude the cimputation phase
		const std::vector<Ptr<Macroflow> >& mfs = job->GetMfList();
		for(uint32_t j = 0;j<mfs.size();++j){
			Ptr<Macroflow> mf = mfs[j];
			mctList.push_back( mf->GetShuffleFinishTime().GetMilliSeconds() - mf->GetStartTime().GetMilliSeconds() ); // note that we exclude the cimputation phase
		}
	}
	ciJctList = m_cis->GetTimeList();
	for(uint32_t i = 0;i<niJctList.size();++i)
		jctList.push_back(niJctList[i]);
	for(uint32_t i = 0;i<ciJctList.size();++i)
		jctList.push_back(ciJctList[i]);

	Log0("");

	OutputSingleMeasurement(niJctList,"nijct");
	OutputSingleMeasurement(ciJctList,"cijct");
	OutputSingleMeasurement(cctList,"cct");
	OutputSingleMeasurement(mctList,"mct");
	OutputSingleMeasurement(jctList,"jct");

	double tp = ciJctList.size() * 1000.0 / m_lastMfStart.GetMilliSeconds();
	Log0( ("cijobs: "+ToString(ciJctList.size())).c_str() );
	Log0( ("tp: "+ToString(tp)).c_str() );
	Log0("");

	double bw = StrBwMb2DblBwMB(GlobalProperty::m_bandwidthMachineToSwitch.Get())*GlobalProperty::m_noMachines;
	//double utility = (m_totalBytes/1000.0/1000.0) / (m_lastMfStart.GetMilliSeconds()/1000.0*bw);
	double utility = (m_totalBytes/1000.0/1000.0) / (Simulator::Now().GetMilliSeconds()/1000.0*bw);
	Log0( ("size(MB): "+ToString(m_totalBytes/1000.0/1000.0)).c_str() );
	Log0( ("utility: "+ToString(utility)).c_str() );
	Log0("");

	Log0( ("max queue length(MB): "+ToString(Queue::max_real_length/1000.0/1000.0)).c_str() );
	double total_packets = m_totalBytes/(double)1460;
	double drop_rate = Queue::dropped_packets/total_packets;
	Log0( ("drop rate: "+ToString(drop_rate)).c_str() );
}

void
MasterApplication::OutputSingleMeasurement(std::vector<uint64_t>& v, std::string prefix) const
{
	std::sort(v.begin(), v.end());
	uint64_t mean = GetMean<uint64_t>(v);
	uint64_t p95 = GetPercentage<uint64_t>(v,0.95);
	uint64_t p99 = GetPercentage<uint64_t>(v,0.99);
	uint64_t p100 = GetPercentage<uint64_t>(v,1.00);

	Log0( (prefix+"-mean: "+ToString(mean)).c_str() );
	Log0( (prefix+"-p95: "+ToString(p95)).c_str() );
	Log0( (prefix+"-p99: "+ToString(p99)).c_str() );
	Log0( (prefix+"-p100: "+ToString(p100)).c_str() );
	Log0("");
}

} /* namespace ns3 */
