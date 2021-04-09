/*
 * machine-slots.h
 *
 *  Created on: Jun 9, 2017
 *      Author: tbc
 */

#ifndef SRC_INTERNET_MF_UTIL_MACHINE_SLOTS_H_
#define SRC_INTERNET_MF_UTIL_MACHINE_SLOTS_H_

#include <iostream>

#include "ns3/ptr.h"
#include "ns3/bulk-send-application.h"
#include "ns3/fatal-error.h"
#include "ns3/application.h"
#include "ns3/macroflow.h"
#include "ns3/global-property.h"
#include "ns3/socket.h"

#include "ns3/tcp-flow-helper.h"
#include "ns3/macroflow.h"
#include "ns3/parallel-job.h"
#include "ns3/global-property.h"
#include <map>

namespace ns3 {

class MachineSlots {
public:
	MachineSlots();
	MachineSlots(uint32_t machineId); // zzc
	virtual ~MachineSlots();

	void AddMf(Ptr<Macroflow> mf);
	void RemoveMf(Ptr<Macroflow> mf);

	//void UpdateFlowStates();
	//void SendFlows(const std::vector<Ptr<Node> >& m_nodeList, const std::vector<Ipv4Address>& m_ipList, uint32_t reducerMachine);


	// eurosys
	void ReallocateFetchers();

	// added by zzc, the map of machineId and point of apps
	std::map<uint32_t, Ptr<BulkSendApplication>> m_flows;


protected:
	std::vector<Ptr<Macroflow> > m_list;
	uint32_t m_countDown;

	double getSpeed() const;

	//uint64_t m_szFinishedMfs;
	//std::vector<uint64_t> m_totSentSize;
	// Z 声明MasterApplication是MachineSlots的友类
	friend class MasterApplication;

private:
	uint32_t m_machineId;
};


} /* namespace ns3 */

#endif /* SRC_INTERNET_MF_UTIL_MACHINE_SLOTS_H_ */
