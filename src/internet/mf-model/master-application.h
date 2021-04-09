/*
 * master-application.h
 *
 *  Created on: Mar 17, 2017
 *      Author: tbc
 */

#ifndef SRC_MACROFLOW_MODEL_MASTER_APPLICATION_H_
#define SRC_MACROFLOW_MODEL_MASTER_APPLICATION_H_


#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>
#include <fstream>

#include "ns3/application.h"
#include "ns3/simulator.h"
#include "ns3/node.h"
#include "ns3/ipv4-header.h"
#include "ns3/bulk-send-application.h"
#include "ns3/red-queue.h"
#include "ns3/rtt-estimator.h"

#include "ns3/tcp-flow-helper.h"
#include "ns3/parallel-job.h"
#include "ns3/macroflow.h"
#include "ns3/trace-reader.h"
#include "ns3/mf-scheduler.h"
#include "ns3/ni-scheduler.h"
#include "ns3/ci-scheduler.h"
#include "ns3/global-property.h"
#include "ns3/machine-slots.h"
#include "ns3/debug-tag.h"
#include "ns3/tracker.h"

#include"ns3/global-property-network.h"


namespace ns3 {


class MasterApplication: public Application {
public:
	MasterApplication();
	virtual ~MasterApplication();
	//void SetNodeList(const std::vector< Ptr<Node> >& nodeList);
	//void SetIpList(const std::vector<Ipv4Address>& ipList);


private:
	virtual void StartApplication ();
	virtual void StopApplication ();
	void LoadTraceFile(std::string traceFile);
	void DoSchedule ();

	void OutputMeasurements() const;
	void OutputSingleMeasurement(std::vector<uint64_t>& v, std::string prefix) const;


	bool m_isStop;
	MfScheduler* m_mfs;
	NiScheduler* m_nis;
	CiScheduler* m_cis;


	std::vector<uint32_t> m_noFreeSlots;

	std::vector<MachineSlots> m_slots;

	Ptr<Application> temp;

	std::vector<Ptr<ParallelJob> > m_jobs;

	uint64_t m_lastClock;

	uint64_t m_totalBytes;
	Time m_lastMfStart;

	uint32_t m_speedEstimatorCounter;
};

} /* namespace ns3 */

#endif /* SRC_MACROFLOW_MODEL_MASTER_APPLICATION_H_ */
