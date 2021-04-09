/*
 * tcp-flow-helper.h
 *
 *  Created on: Mar 17, 2017
 *      Author: tbc
 */

#ifndef SRC_MACROFLOW_HELPER_TCP_FLOW_HELPER_H_
#define SRC_MACROFLOW_HELPER_TCP_FLOW_HELPER_H_

//#include <vector>

#include "ns3/application-container.h"
#include "ns3/bulk-send-helper.h"
#include "ns3/bulk-send-application.h"
#include "ns3/inet-socket-address.h"
#include "ns3/ipv4-address.h"
#include "ns3/node.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"
#include "ns3/nstime.h"
#include "ns3/application.h"

#include "ns3/global-property.h"

namespace ns3 {


class TcpFlowHelper
{
public:

	static void InstallSinkApp(Ptr<Node> sinkNode);
	static Ptr<BulkSendApplication> CreatePendingTcpFlow(Ptr<Node> from, Ipv4Address to, uint32_t bytes, uint8_t dscp, const Time& basicDelay);

	static inline void UpdateDscp(Ptr<BulkSendApplication> bulkSendApplication, uint8_t dscp) { bulkSendApplication->SetAttribute("TOS", UintegerValue (dscp)); }
	static inline void EstimateSpeed(Ptr<BulkSendApplication> bulkSendApplication) { bulkSendApplication->EstimateCurrentSpeed(); }

	static uint16_t m_serverPort;
};


}


#endif /* SRC_MACROFLOW_HELPER_TCP_FLOW_HELPER_H_ */
