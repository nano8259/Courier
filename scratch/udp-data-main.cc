/*
 * udp-data-main.cc
 *
 *  Created on: 2018年7月13日
 *      Author: kathy
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
#include "ns3/switch-helper.h"

using namespace ns3;
using namespace boost;

std::string pathOut = ".";

double app_start_time = 1;
double app_stop_time = 8.5;
double start_time = 2;
double stop_time = 8;

int main (int argc, char *argv[])
{

	LogComponentEnable ("BulkSendApplication", LOG_LEVEL_WARN);
	//LogComponentEnable ("TcpL4Protocol", LOG_LEVEL_INFO);
	LogComponentEnable ("TcpSocketBase", LOG_LEVEL_WARN);
	//LogComponentEnable ("TcpSocket", LOG_LEVEL_INFO);
	//LogComponentEnable ("RedQueue", LOG_LEVEL_INFO);
	//LogComponentEnable ("Ipv4Interface", LOG_LEVEL_INFO);

	//for debug
	//std::string config_file = "config.ini";
	//std::string output_file = "lossrate_train.txt";
	//GlobalProperty::LoadSettings(config_file);
	//GlobalProperty::m_log.Start(output_file);

	//std::string config_file = "config.ini";
	//std::string output_file = "LossRate.txt";
	GlobalProperty::LoadSettings(argv[1]);
	GlobalProperty::m_log.Start(argv[2]);

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

	uint64_t flow_num = 60;
	//uint64_t min_queue_length = 90;
	//uint64_t max_queue_length = 200;
	//Time interPacketInterval = Seconds(0.000012);
	Time interPacketInterval = Seconds(0.00006);//1/5 bandwidth
	RngSeedManager::SetSeed(time(0));
	//std::cout<<time(0)<<std::endl;
	//uint64_t queue_length[10]={100,150,200,250,300,350,400,450,500,550};

	// Z 迭代
	for(uint64_t k=0; k < GlobalProperty::m_train_num; k++){
		GlobalProperty::m_round = k;
		int length_tag = 0;

		int queue_length[10] = {100,150,200,250,300,350,400,450,500,550};

		int tag = int(UniformVariable(0,1).GetValue()*(GlobalProperty::m_class_num));
		/*if(tag < 0.5)
			(GlobalProperty::m_redLength).Set(300);
			length_tag = 1;
		}
		else{
			(GlobalProperty::m_redLength).Set(100);
			length_tag = 0;
		}*/
		// Z 这个tag决定了red队列的长度
		(GlobalProperty::m_redLength).Set(queue_length[tag]);
		length_tag = tag;


		NodeContainer clientNodes;
		clientNodes.Create (GlobalProperty::m_noMachines);
		InternetStackHelper().Install (clientNodes);
		for(uint64_t i=k*clientNodes.GetN(); i<(k+1)*clientNodes.GetN(); ++i)
			Names::Add ((format("/Names/Client%d") % (i)).str(), clientNodes.Get (i-k*clientNodes.GetN()));


		//build topology
		TopologyHelper topology;
		Ptr<Node> masterNode = topology.Build(clientNodes);//switch node


		// add udp application

		//Ptr<MasterApplication> master = Create<MasterApplication>();
		/*master->SetNodeList(sw.GetNodeList());
		master->SetIpList(sw.GetIpList());
		master->SetStartTime(Seconds(0));
		sw.GetNode()->AddApplication(master);*/
		GlobalProperty::m_nodeList = topology.GetNodeList();
		GlobalProperty::m_ipList = topology.GetIpList();

		// Z 初始化一些数据
		int totalPacket = 0;
		GlobalPropertyNetwork::m_noReceived = 0;
		bool used_port[65536] = {0};
		// Z 流的数量在上面规定了，目前60
		for(uint64_t i=0; i<flow_num; i++){
			int maxPacket = 10;//short flow:100
			totalPacket += maxPacket;
			//std::cout<<(GlobalProperty::m_redLength).Get()<<std::endl;
			int src,dst;
			// Z 这样子的话是随机发起流
			/*while(1){
				src = int(UniformVariable(0,1).GetValue()*GlobalProperty::m_noMachines);
				dst = int(UniformVariable(0,1).GetValue()*GlobalProperty::m_noMachines);
				dst = 16;
				if (src != dst)
					break;
			}*/
			// Z 这样子的话是大家都向15号发起流
			src = i%15;
			dst = 15;

			int port;
			// Z port是0-65535，所以后面乘40000，不过乘65535不是更好？
			// Z 为啥不直接为Uniform...给个上下界？
			while (used_port[port = int(UniformVariable(0, 1).GetValue() * 40000)])
				continue;
			used_port[port] = true;
			// Z 创建sever
			Ipv4Address serverAddress = GlobalProperty::m_ipList[dst];
			UdpServerHelper server0(port);
			ApplicationContainer app0 = server0.Install(clientNodes.Get(dst));

			app0.Start(Seconds(app_start_time));
			app0.Stop(Seconds(app_stop_time));
			UdpClientHelper client0(serverAddress, port);
			client0.SetAttribute("MaxPackets", UintegerValue(maxPacket));
			client0.SetAttribute("Interval", TimeValue(interPacketInterval));
			client0.SetAttribute("PacketSize", UintegerValue(pktSize));

			ApplicationContainer apps0c = client0.Install(clientNodes.Get(src));
			// Z TODO 这个shift是干啥？
			double shift = UniformVariable(0, 1).GetValue()*0.00006;
			//std::cout<<shift<<std::endl;
			apps0c.Start(Seconds(start_time+shift));
			apps0c.Stop(Seconds(stop_time+shift));

			}


		// pcap configuration
		if(GlobalProperty::m_pcapOutput){
			std::stringstream pcapStr;
			pcapStr << pathOut << "/mf";
			PointToPointHelper().EnablePcapAll (pcapStr.str());
		}


		// run simulation

		//clock_t t1 = clock();
		Simulator::Run ();
		Simulator::Destroy ();
		//clock_t t2 = clock();

		//Log1("Simulation Time: %ss",ToString((t2-t1)/CLOCKS_PER_SEC).c_str());
		//double unforced_drop_rate = 100*GlobalPropertyNetwork::m_noPacketLoss[0]/double(totalPacket);
		//double forced_drop_rate = 100*GlobalPropertyNetwork::m_noPacketLoss[1]/double(totalPacket);
		//double queue_limits_drop_rate = 100*GlobalPropertyNetwork::m_noPacketLoss[2]/double(totalPacket);
		//double total_drop_rate1 = unforced_drop_rate + forced_drop_rate + queue_limits_drop_rate;
		//std::cout<<GlobalPropertyNetwork::m_noPacketLoss[0] << " " <<GlobalPropertyNetwork::m_noPacketLoss[1] << " " <<GlobalPropertyNetwork::m_noPacketLoss[2]<<std::endl;
		//std::cout<<totalPacket<<" "<<GlobalPropertyNetwork::m_noReceived<<std::endl;
		//std::cout<<totalPacket - GlobalPropertyNetwork::m_noReceived << std::endl;
		double total_drop_rate = (totalPacket - GlobalPropertyNetwork::m_noReceived)/double(totalPacket);
		//std::cout<<unforced_drop_rate << " " <<forced_drop_rate << " " <<queue_limits_drop_rate <<" "<<total_drop_rate<<std::endl;
		//Log4("loss rate unforced %.3f, forced %.3f, queuelimits %.3f, total %.3f ", unforced_drop_rate, forced_drop_rate, queue_limits_drop_rate,total_drop_rate);
		Log2("%d 1: %f",length_tag,total_drop_rate);
		//std::cout<<k<<" total packet "<<totalPacket<<std::endl;
		//std::cout<<length_tag<<" 1:"<<total_drop_rate<<std::endl;

	}
	std::cout<<"done."<<std::endl;
	return 0;
}



