/*
 * global-property.h
 *
 *  Created on: Mar 21, 2017
 *      Author: tbc
 */

#ifndef SRC_MACROFLOW_UTIL_GLOBAL_PROPERTY_H_
#define SRC_MACROFLOW_UTIL_GLOBAL_PROPERTY_H_

#include <cstdio>
#include <stdint.h>
#include <string>
#include <algorithm>
#include <vector>
#include <numeric>
#include <sstream>
#include <cstring>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "ns3/nstime.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/node.h"
#include "ns3/ipv4-address.h"

#include "ns3/global-log.h"


#define Log0(a) do{GlobalProperty::m_log.PrintLine(a);}while(false);
#define Log1(a,b) do{char buffer[64];sprintf(buffer,a,b);GlobalProperty::m_log.PrintLine(buffer);}while(false);
#define Log2(a,b,c) do{char buffer[64];sprintf(buffer,a,b,c);GlobalProperty::m_log.PrintLine(buffer);}while(false);
#define Log3(a,b,c,d) do{char buffer[64];sprintf(buffer,a,b,c,d);GlobalProperty::m_log.PrintLine(buffer);}while(false);
#define Log4(a,b,c,d,e) do{char buffer[128];sprintf(buffer,a,b,c,d,e);GlobalProperty::m_log.PrintLine(buffer);}while(false);

namespace ns3 {

class GlobalProperty {
public:
	GlobalProperty() {}
	virtual ~GlobalProperty() {}

	static void LoadSettings(std::string file);

	// predefined settings
	static uint32_t m_noMachines;
	static uint32_t m_noSlots;
	static StringValue m_bandwidthMachineToSwitch;
	static StringValue m_bandwidthSwitchToSwitch;
	static uint32_t m_noQueues;
	static uint32_t m_noRackSwitches;

	static std::string m_traceFile;
	static double m_sizeScale;
	static double m_timeScale;

	static std::string m_niSchedulerType;
	static std::string m_ciSchedulerType;
	static std::string m_mfSchedulerType;
	static std::string m_mfSchedulerSubtype;

	static UintegerValue m_redLength;
	static DoubleValue m_redThreshold;

	static Time m_ciDuration;
	static Time m_niDuration;
	static bool m_periodic;
	static uint64_t m_periods;
	static uint32_t m_totalCi;




	static Time m_simulationStep;
	static bool m_pcapOutput;
	static uint32_t m_seed;
	static bool m_randomMapping;
	static std::string m_scheduler;

	static uint32_t m_reducerMaxDelayUs;
	static uint32_t m_flowMaxDelayUs;

	static double m_speedEstimatorEma;
	static uint32_t m_speedEstimatorPeriod; // times of simulator-step
	static bool m_uniformSlotAllocation;

	static uint32_t m_maxFetchers;
	static bool m_bwAlgorithm;
	static uint32_t m_bwAlgorithmDetails;
	static uint32_t m_senderPromotion;

	// dynamic properties
	static uint32_t m_noEmittedNiJobs;
	static uint32_t m_noTotalNiJobs; // [W]=TraceReader::Analysis [R]=
	static uint32_t m_noFinishedNiJobs;

	static GlobalLog m_log;

	static void* m_receiverSocket[160][66000];

	static std::vector< Ptr<Node> > m_nodeList;
	static std::vector<Ipv4Address> m_ipList;

	//added by lkx, for packet loss analysis
	static uint64_t m_TotalBytes;
	static uint64_t m_round;
	static uint64_t m_train_num;
	static bool m_dropTailMode;
	static uint64_t m_class_num;

	//parameter
	static uint32_t m_speed_limit;
	static double m_connection_limit;
	static uint32_t m_speed_limit_tracker;
	static bool m_log_state;

	//for test
	static uint64_t m_totalSendBytes;
	static uint64_t m_totalSendBytes_all;
	static uint64_t m_count_test;

	//lkx
	static bool m_ack_priority;
	static bool m_adjust_priority;

};

void ToLower(std::string& str);
uint32_t SumVec(const std::vector<uint32_t>& vec);
uint32_t MaxPos(const std::vector<uint32_t>& vec);
uint32_t FirstPositive(const std::vector<uint32_t>& vec);
Time TimeDiff(const Time& a, const Time& b);
double StrBwMb2DblBwMB(const std::string& bw);

inline uint32_t GetDigit(uint32_t number,uint32_t divisor){ return (number/divisor)%10; }

template<typename T> std::string ToString(T num){ std::stringstream ss; ss<<num; std::string str; ss>>str; return str; }
template<typename T> T GetMean(std::vector<T> v){ T s = 0; for(uint32_t i=0;i<v.size();++i) s+=v[i]; return v.empty()?0:s/v.size(); }
template<typename T> T GetPercentage(std::vector<T> v, double p){ return v.empty()?0:v[static_cast<uint32_t>((v.size()-1)*p)]; }
static inline uint32_t makepair(uint32_t jobId, uint32_t reducerId) { return (jobId<<16) + reducerId; }
} /* namespace ns3 */

#endif /* SRC_MACROFLOW_UTIL_GLOBAL_PROPERTY_H_ */
