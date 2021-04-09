/*
 * global-property.cc
 *
 *  Created on: Mar 21, 2017
 *      Author: tbc
 */


#include "global-property.h"

namespace ns3 {


void
GlobalProperty::LoadSettings(std::string file)
{
	boost::property_tree::ptree root, config;
	boost::property_tree::read_ini(file, root);

	// [0] simulator
	config = root.get_child("simulator");
	m_simulationStep = MilliSeconds(config.get<uint32_t>("simulation-step")); // default: 5ms;
	m_log.SetScreenOutput(config.get<bool>("screen-output")); // default: true
	m_pcapOutput = config.get<bool>("pcap-output"); // default: true
	m_seed = config.get<uint32_t>("seed");// default: 666
	m_scheduler = config.get<std::string>("event-scheduler"); // default: map

	// [1] topology
	config = root.get_child("topology");
	m_noMachines = config.get<uint32_t>("number-of-machines"); // default: 16
	m_noSlots = config.get<uint32_t>("number-of-slots"); // default: 4
	m_noQueues = config.get<uint32_t>("number-of-queues"); // default: 8
	if(m_noQueues > 8)
		NS_FATAL_ERROR("At most 8 queues!");
	m_redLength = UintegerValue(config.get<uint32_t>("red-length")); // default: 90
	m_redThreshold = DoubleValue(static_cast<double>(config.get<uint32_t>("red-threshold"))); // default: 10
	m_bandwidthMachineToSwitch = StringValue(config.get<std::string>("bandwidth-machine-to-switch")); // default: 1000Mbps
	m_bandwidthSwitchToSwitch = StringValue(config.get<std::string>("bandwidth-switch-to-switch")); // default: 8000Mbps
	m_noRackSwitches = config.get<uint32_t>("number-of-rack-switches"); // defalut: 1

	// [2] trace
	config = root.get_child("trace");
	m_traceFile = config.get<std::string>("trace-file");
	m_sizeScale = config.get<double>("size-scale"); // default: 0.1
	m_timeScale = config.get<double>("time-scale"); // default: 1.0
	m_randomMapping = config.get<bool>("random-mapping"); // default: true

	// [3] scheduler
	config = root.get_child("scheduler");
	// [3.1] ni scheduler
	m_niSchedulerType = config.get<std::string>("ni-scheduler-type");// [fair] or size
	m_niDuration = MilliSeconds(config.get<uint32_t>("ni-computation-duration")); // default: 0ms;
	// [3.2] ci scheduler
	m_ciSchedulerType = config.get<std::string>("ci-scheduler-type");// [oppo(rtunistic)] or peri(odic)
	m_ciDuration = MilliSeconds(config.get<uint32_t>("ci-computation-duration")); // default: 10000ms;
	m_periodic = config.get<bool>("ci-periodic"); // default: false
	m_periods = config.get<uint32_t>("ci-periods"); // default: 500ms (available when m_periodic==true)
	m_totalCi = config.get<uint32_t>("ci-total"); // (available when m_periodic==true)

	// [3.3] mf scheduler
	m_mfSchedulerType = config.get<std::string>("mf-scheduler-type"); // smf scf mlas clas
	m_mfSchedulerSubtype = config.get<std::string>("mf-scheduler-subtype"); // our exp uni
	m_reducerMaxDelayUs = config.get<uint32_t>("mf-reducer-max-delay") * 1000;
	m_flowMaxDelayUs = config.get<uint32_t>("mf-flow-max-delay") * 1000;
	m_maxFetchers = config.get<uint32_t>("max-fetchers"); // how much hosts can a reducer connect with at most?

	// [4] eurosys
	config = root.get_child("eurosys");
	m_speedEstimatorEma = config.get<double>("speed-estimator-ema"); // default: 1/16
	m_speedEstimatorPeriod = config.get<uint32_t>("speed-estimator-period"); // default: 20ms, times of simulator-step
	m_uniformSlotAllocation = config.get<bool>("uniform-slot-allocation"); // only for debugging, default: true
	m_bwAlgorithm = config.get<bool>("bw-algorithm");
	m_bwAlgorithmDetails = config.get<uint32_t>("bw-algorithm-details");
	m_senderPromotion = config.get<uint32_t>("sender-promotion");
	m_train_num = config.get<uint64_t>("train-num");
	m_dropTailMode = config.get<bool>("drop-tail-mode");
	m_class_num = config.get<uint64_t>("class-num");
	m_speed_limit = config.get<uint32_t>("speed-limit");
	m_speed_limit_tracker = config.get<uint32_t>("speed-limit-tracker");
	m_connection_limit = config.get<double>("connection-limit");
	m_log_state = config.get<bool>("log-state");
	m_ack_priority = config.get<bool>("ack-priority");
	m_adjust_priority = config.get<bool>("adjust-priority");

	// courier
	config = root.get_child("courier");
	std::string mode_string = config.get<std::string>("mode");
	if(mode_string == "fair"){
		m_courier_mode = GlobalProperty::CourierMode::fair;
	}else if(mode_string == "fifo"){
		m_courier_mode = GlobalProperty::CourierMode::fifo;
	}else if(mode_string == "sf"){
		m_courier_mode = GlobalProperty::CourierMode::sf;
	}else if(mode_string == "schedule"){
		m_courier_mode = GlobalProperty::CourierMode::schedule;
	}
	std::cout << m_courier_mode << std::endl; // for debug

	// init
	memset(m_receiverSocket,0,sizeof(m_receiverSocket));
}

// predefined settings
uint32_t GlobalProperty::m_noMachines;
uint32_t GlobalProperty::m_noSlots;

std::string GlobalProperty::m_traceFile;
double GlobalProperty::m_sizeScale;
double GlobalProperty::m_timeScale;
uint32_t GlobalProperty::m_noQueues;
StringValue GlobalProperty::m_bandwidthMachineToSwitch;
StringValue GlobalProperty::m_bandwidthSwitchToSwitch;
uint32_t GlobalProperty::m_noRackSwitches;

std::string GlobalProperty::m_niSchedulerType;
std::string GlobalProperty::m_ciSchedulerType;
std::string GlobalProperty::m_mfSchedulerType;
std::string GlobalProperty::m_mfSchedulerSubtype;

UintegerValue GlobalProperty::m_redLength;
DoubleValue GlobalProperty::m_redThreshold;

Time GlobalProperty::m_ciDuration;
Time GlobalProperty::m_niDuration;
bool GlobalProperty::m_periodic;
uint64_t GlobalProperty::m_periods;
uint32_t GlobalProperty::m_totalCi;



Time GlobalProperty::m_simulationStep;
bool GlobalProperty::m_pcapOutput;
uint32_t GlobalProperty::m_seed;
bool GlobalProperty::m_randomMapping;
std::string GlobalProperty::m_scheduler;

uint32_t GlobalProperty::m_reducerMaxDelayUs;
uint32_t GlobalProperty::m_flowMaxDelayUs;

// eurosys
double GlobalProperty::m_speedEstimatorEma;
uint32_t GlobalProperty::m_speedEstimatorPeriod;
bool GlobalProperty::m_uniformSlotAllocation;

uint32_t GlobalProperty::m_maxFetchers;
bool GlobalProperty::m_bwAlgorithm;
uint32_t GlobalProperty::m_bwAlgorithmDetails;
uint32_t GlobalProperty::m_senderPromotion;

// dynamic properties
uint32_t GlobalProperty::m_noEmittedNiJobs = 0;
uint32_t GlobalProperty::m_noTotalNiJobs = 0;
uint32_t GlobalProperty::m_noFinishedNiJobs = 0;

GlobalLog GlobalProperty::m_log;

void* GlobalProperty::m_receiverSocket[160][66000];


std::vector< Ptr<Node> > GlobalProperty::m_nodeList;
std::vector<Ipv4Address> GlobalProperty::m_ipList;

//added by lkx, for packet loss analysis
uint64_t GlobalProperty::m_TotalBytes;
uint64_t GlobalProperty::m_round;
uint64_t GlobalProperty::m_train_num;
bool GlobalProperty::m_dropTailMode;
uint64_t GlobalProperty::m_class_num;

uint32_t GlobalProperty::m_speed_limit;
double GlobalProperty::m_connection_limit;
uint32_t GlobalProperty::m_speed_limit_tracker;
bool GlobalProperty::m_log_state;

//for test
uint64_t GlobalProperty::m_totalSendBytes = 0;
uint64_t GlobalProperty::m_totalSendBytes_all = 0;
uint64_t GlobalProperty::m_count_test = 0;

bool GlobalProperty::m_ack_priority;
bool GlobalProperty::m_adjust_priority;

// courier zczhang
GlobalProperty::CourierMode GlobalProperty::m_courier_mode;

// fucntions
void ToLower(std::string& str) {
	// 对每一个元素执行tolower操作
	std::transform(str.begin(),str.end(),str.begin(),static_cast<int(*)(int)>(&std::tolower));
}

uint32_t SumVec(const std::vector<uint32_t>& vec) {
	return std::accumulate(vec.begin(),vec.end(),static_cast<uint32_t>(0));
}


uint32_t MaxPos(const std::vector<uint32_t>& vec) {
	uint32_t pos = 0;
	for(uint32_t i=0;i<vec.size();++i)
		if(vec[i]>vec[pos])
			pos = i;
	return pos;
}

uint32_t FirstPositive(const std::vector<uint32_t>& vec){
	for(uint32_t i=0;i<vec.size();++i)
		if(vec[i]>0)
			return i;
	return vec.size();
}

Time TimeDiff(const Time& a, const Time& b) {
	int64_t ta = static_cast<int64_t>(a.GetMicroSeconds());
	int64_t tb = static_cast<int64_t>(b.GetMicroSeconds());
	uint64_t diff = static_cast<uint64_t>(std::abs(ta-tb));
	return MicroSeconds(diff);
}

double StrBwMb2DblBwMB(const std::string& bw) {
	double ans=0;
	for(uint32_t i=0;i<bw.length()&&isdigit(bw[i]);++i)
		ans = 10*ans + (bw[i]-'0');
	return ans/8;
}


} /* namespace ns3 */
