/*
 * Tracker.h
 *
 *  Created on: Oct 7, 2017
 *      Author: tbc
 */

#ifndef SRC_INTERNET_MF_MODEL_TRACKER_H_
#define SRC_INTERNET_MF_MODEL_TRACKER_H_

#include <vector>
#include <algorithm>
#include <map>

#include "ns3/global-property.h"
#include "ns3/macroflow.h"
#include "ns3/parallel-job.h"

namespace ns3 {

class SenderInfo{
public:
	double bw;
	//uint64_t size;
	uint64_t timeStamp;
	SenderInfo(double BW=0){ bw = BW; timeStamp = (uint64_t)-1; }
	//inline bool IsFinished() { return size==0;}
};

class Tracker { // static class
public:
	Tracker() {}
	virtual ~Tracker() {}

	static bool initialized;

	// bandwidth
	static std::vector<double> uLink;
	static std::vector<double> dLink;
	static std::vector<uint32_t> uConn;
	static std::vector<Ptr<ParallelJob> > uPrio; // prioritized coflow in sender end

	static SenderInfo sender[600][160];// i-th coflow(job), j-th mapper machine
	static SenderInfo receiver[600][160];// i-th coflow(job), j-th mapper machine

	static void UpdateBandwidth(const std::vector<Ptr<Macroflow> >& mfs);

	static void ReportMapperList(Ptr<Macroflow> reducer);
	static std::vector<uint32_t> GetCandidateList(Ptr<Macroflow> reducer, bool prioritized) ; // cannot return quoted vector because it's a local variable
	static void ReportChosenHosts(Ptr<Macroflow> reducer, const std::vector<uint32_t>& chosenSender);
	static void ReportPausedHosts(Ptr<Macroflow> reducer, const std::vector<uint32_t>& pausedSender);

private:
	static void init();
	// Z 两个十六位合进一个三十二位
	static inline uint32_t makepair(uint32_t jobId, uint32_t reducerId) { return (jobId<<16) + reducerId; }
	// Z 读高16位
	static inline uint32_t getJobId(uint32_t pair)  { return pair>>16; }
	// Z 读低16位
	static inline uint32_t getReducerId(uint32_t pair)  { return pair & 0xffff;}

	// remaining sender machine list
	static std::map<uint32_t,std::vector<uint32_t> > senderList;  // k(job-reducer pair) v(sender-machine list)
};

} /* namespace ns3 */

#endif /* SRC_INTERNET_MF_MODEL_TRACKER_H_ */
