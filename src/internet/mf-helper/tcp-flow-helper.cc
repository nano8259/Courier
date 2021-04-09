/*
 * tcp-flow-helper.cc
 *
 *  Created on: Mar 17, 2017
 *      Author: tbc
 */

#include "tcp-flow-helper.h"

namespace ns3 {

uint16_t TcpFlowHelper::m_serverPort = 9;

void
TcpFlowHelper::InstallSinkApp(Ptr<Node> sinkNode)
{
	// Z TODO 这个sink的作用以及输出还没搞懂
	PacketSinkHelper sink ("ns3::TcpSocketFactory",
			InetSocketAddress (Ipv4Address::GetAny (), m_serverPort));
	ApplicationContainer sinkApps = sink.Install (sinkNode);
	sinkApps.Start (Seconds(0));
}

Ptr<BulkSendApplication>
TcpFlowHelper::CreatePendingTcpFlow(Ptr<Node> from, Ipv4Address to, uint32_t bytes, uint8_t dscp, const Time& basicDelay)
{
	BulkSendHelper source ("ns3::TcpSocketFactory",
			InetSocketAddress (to, m_serverPort));
	source.SetAttribute ("MaxBytes", UintegerValue (bytes));
	source.SetAttribute ("TOS", UintegerValue (dscp));
	ApplicationContainer sourceApps = source.Install (from);
	Ptr<BulkSendApplication> app = Ptr<BulkSendApplication>( (BulkSendApplication*) (sourceApps.Get(0).operator ->()) );
	// Z TODO 
	app->InitiallizeSpeedEstimator();
	return app;
}


} // namespace ns3


