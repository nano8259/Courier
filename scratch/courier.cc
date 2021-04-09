/*
 * courier.cc
 *
 *  Created on: Jan 6, 2021
 *      Author: zczhang
 */


#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <iomanip>
#include <ctime>

#include <boost/format.hpp>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/applications-module.h"

using namespace ns3;
using namespace boost;

std::string pathOut = ".";

int
main (int argc, char *argv[])
{

	for(int i=0;i<argc;++i)
		std::cout<< argv[i] <<std::endl;
	std::cout<<std::endl;

	LogComponentEnable ("BulkSendApplication", LOG_LEVEL_WARN);
	//LogComponentEnable ("TcpL4Protocol", LOG_LEVEL_INFO);
	LogComponentEnable ("TcpSocketBase", LOG_LEVEL_WARN);
	//LogComponentEnable ("TcpSocket", LOG_LEVEL_INFO);
	//LogComponentEnable ("RedQueue", LOG_LEVEL_INFO);
	//LogComponentEnable ("Ipv4Interface", LOG_LEVEL_INFO);

	if(argc!=3)
		NS_FATAL_ERROR("\n Wrong command parameters!\n ./waf --run \"macroflow_main <setting file> <trace file> <log file>\"\n" );
	GlobalProperty::LoadSettings(argv[1]);
	// GlobalProperty::m_traceFile = (argv[2]);
	// std::cout << "start log" << std::endl;
	GlobalProperty::m_log.Start(argv[2]);
	// for debug
	// std::string config_file = "config.ini";
	// std::string output_file = "courier-output.txt";
	// GlobalProperty::LoadSettings(config_file);
	// GlobalProperty::m_log.Start(output_file);


	// fixed property

	//const uint32_t maxPackets = 90;
	const uint32_t pktSize = 1460;


	// set simulation environments

	Config::SetDefault ("ns3::BulkSendApplication::SendSize", UintegerValue (pktSize));
	Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (pktSize));
	Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (1)); // default: 2
	Config::SetDefault ("ns3::TcpSocket::InitialCwnd", UintegerValue (2));// default: 1
	Config::SetDefault ("ns3::TcpSocketBase::DCTCP", BooleanValue (true)); // default: false
	Config::SetDefault ("ns3::RedQueue::UseCurrent", BooleanValue (true)); // default: false
	Config::SetDefault ("ns3::TcpSocket::ECN", BooleanValue (true));// default: false
	Config::SetDefault ("ns3::RedQueue::NoQueues", UintegerValue (8)); // default: 8  NOTE: define as 8, but now always use
	Config::SetDefault ("ns3::RedQueue::DRR", BooleanValue (false)); // default: true
	Config::SetDefault ("ns3::RedQueue::HeadDrop", BooleanValue (false)); // default: true
	//Config::SetDefault ("ns3::TcpSocket::ConnTimeout", StringValue ("200ms"));  // default: 3s
    //Config::SetDefault ("ns3::TcpSocket::ConnCount", UintegerValue(6)); // default: 6
	Config::SetDefault ("ns3::TcpSocketBase::UserRTO", TimeValue (MilliSeconds (2)));// default: 0.2s
    Config::SetDefault ("ns3::TcpSocketBase::AckDSCP", UintegerValue (Ipv4Header::CS7)); // default: CS0
	Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpReno")); // default TcpNewreno
    GlobalValue::Bind ("ChecksumEnabled", BooleanValue (false)); // otherwise ECN modification lead to a drop in L3

	ToLower(GlobalProperty::m_scheduler); // default: map
	ObjectFactory scheduler;
	if(GlobalProperty::m_scheduler.compare("map")==0) scheduler.SetTypeId(MapScheduler::GetTypeId ());
	else if(GlobalProperty::m_scheduler.compare("heap")==0) scheduler.SetTypeId(HeapScheduler::GetTypeId ());
	else if(GlobalProperty::m_scheduler.compare("list")==0) scheduler.SetTypeId(ListScheduler::GetTypeId ());
	else NS_FATAL_ERROR("Unknown simulator scheduler: "+ GlobalProperty::m_scheduler);
	Simulator::SetScheduler(scheduler);
	// create client nodes
	NodeContainer clientNodes;
	clientNodes.Create (GlobalProperty::m_noMachines);
	InternetStackHelper().Install (clientNodes);
	for(uint32_t i=0; i<clientNodes.GetN(); ++i)
		Names::Add ((format("/Names/Client%d") % (i)).str(), clientNodes.Get (i));

	// create switch node

	/*SwitchHelper sw;
	Names::Add ("/Names/Switch", sw.GetNode());
	sw.SetIngressQueue ("ns3::DropTailQueue", "MaxPackets", GlobalProperty::m_redLength);
	//sw.SetEgressQueue ("ns3::DropTailQueue", "MaxPackets", GlobalProperty::m_redLength);
	sw.SetEgressQueue ("ns3::RedQueue", "MinTh",GlobalProperty::m_redThreshold, "MaxTh", GlobalProperty::m_redThreshold, "QueueLimit", GlobalProperty::m_redLength);
	sw.SetLinkDelay(StringValue ("0.05ms"));
	sw.SetDataRate(GlobalProperty::m_bandwidthMachineToSwitch);
	for(uint32_t i = 0;i<clientNodes.GetN();++i)
		sw.AddClientNode(clientNodes.Get (i));
	sw.Install();*/
	TopologyHelper topology;
	Ptr<Node> masterNode = topology.Build(clientNodes);
	GlobalProperty::m_nodeList = topology.GetNodeList();
	// Log1("size of node list %d", topology.GetNodeList().size());
	GlobalProperty::m_ipList = topology.GetIpList();
	// add master application
	Ptr<MasterApplication> master = Create<MasterApplication>();
	/*master->SetNodeList(sw.GetNodeList());
	master->SetIpList(sw.GetIpList());
	master->SetStartTime(Seconds(0));
	sw.GetNode()->AddApplication(master);*/
	std::cout<<"set start time"<<std::endl;
	master->SetStartTime(Seconds(0));
	masterNode->AddApplication(master);
	// 脚本很简单，主要的问题在于理解MasterApplication

	// pcap configuration
	if(GlobalProperty::m_pcapOutput){
		std::stringstream pcapStr;
		pcapStr << pathOut << "/mf";
		PointToPointHelper().EnablePcapAll (pcapStr.str());
	}


	// run simulation

	//Simulator::Stop (Seconds (50));
	clock_t t1 = clock();
	Simulator::Run ();
	Simulator::Destroy ();
	clock_t t2 = clock();

	//std::cout<<"Done."<<std::endl;
	Log1("Simulation Time: %ss",ToString((t2-t1)/CLOCKS_PER_SEC).c_str());
	std::cout<<"done "<<argv[2]<<std::endl;

	return 0;
}
