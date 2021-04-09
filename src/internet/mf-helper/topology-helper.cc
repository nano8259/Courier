/*
 * topology-helper.cc
 *
 *  Created on: Apr 7, 2017
 *      Author: tbc
 */

#include "topology-helper.h"

namespace ns3 {

TopologyHelper::TopologyHelper() {
	// TODO Auto-generated constructor stub

}

TopologyHelper::~TopologyHelper() {
	// TODO Auto-generated destructor stub
}

Ptr<Node>
TopologyHelper::Build(NodeContainer& clientNodes)
{
	if(GlobalProperty::m_dropTailMode == true)
		BuildDropTail(clientNodes);
	else if(GlobalProperty::m_noRackSwitches == 1)
		BuildSingle(clientNodes);
	else
		BuildTree(clientNodes);
	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
	return m_root.GetNode();
}

std::vector< Ptr<Node> >
TopologyHelper::GetNodeList() const
{
	if(GlobalProperty::m_noRackSwitches == 1)
		return m_root.GetNodeList();

	std::vector< Ptr<Node> > temp;
	for(uint32_t i=0;i<m_switches.size();++i){
		std::vector< Ptr<Node> > v = m_switches[i].GetNodeList();
		for(uint32_t j=0;j<v.size();++j)
			temp.push_back(v[j]);
	}
	return temp;
}

std::vector<Ipv4Address>
TopologyHelper::GetIpList() const
{
	if(GlobalProperty::m_noRackSwitches == 1)
		return m_root.GetIpList();

	std::vector<Ipv4Address> temp;
	for(uint32_t i=0;i<m_switches.size();++i){
		std::vector<Ipv4Address> v = m_switches[i].GetIpList();
		for(uint32_t j=0;j<v.size();++j)
			temp.push_back(v[j]);
	}
	return temp;
}

// Z 创建一个以一个switch为中心的拓扑，使用droptail队列
void
TopologyHelper::BuildDropTail(NodeContainer& clientNodes)
{
	//Names::Add ("/Names/RootSwitch", m_root.GetNode());
	// Z m_round 每轮迭代都需要创建一个名字不同的中心switch
	Names::Add ((boost::format("/Names/RootSwitch%d") % (GlobalProperty::m_round)).str(), m_root.GetNode());
	//m_root.SetIngressQueue ("ns3::DropTailQueue", "MaxPackets", GlobalProperty::m_redLength);
	//m_root.SetEgressQueue ("ns3::DropTailQueue", "MaxPackets", GlobalProperty::m_redLength);
	//m_root.SetEgressQueue ("ns3::RedQueue", "MinTh",GlobalProperty::m_redThreshold, "MaxTh", GlobalProperty::m_redThreshold, "QueueLimit", GlobalProperty::m_redLength);
	m_root.SetLinkDelay(StringValue ("0.04ms"));
	m_root.SetDataRate(GlobalProperty::m_bandwidthMachineToSwitch);
	for(uint32_t i = 0;i<clientNodes.GetN();++i)
		m_root.AddClientNode(clientNodes.Get (i));
	m_root.Install("ns3::DropTailQueue","MaxPackets", GlobalProperty::m_redLength);
}

// Z 创建一个以一个switch为中心的拓扑，使用red队列
void
TopologyHelper::BuildSingle(NodeContainer& clientNodes)
{
	//Names::Add ("/Names/RootSwitch", m_root.GetNode());
	Names::Add ((boost::format("/Names/RootSwitch%d") % (GlobalProperty::m_round)).str(), m_root.GetNode());
	//m_root.SetIngressQueue ("ns3::DropTailQueue", "MaxPackets", GlobalProperty::m_redLength);
	//m_root.SetEgressQueue ("ns3::DropTailQueue", "MaxPackets", GlobalProperty::m_redLength);
	//m_root.SetEgressQueue ("ns3::RedQueue", "MinTh",GlobalProperty::m_redThreshold, "MaxTh", GlobalProperty::m_redThreshold, "QueueLimit", GlobalProperty::m_redLength);
	m_root.SetLinkDelay(StringValue ("0.04ms"));
	m_root.SetDataRate(GlobalProperty::m_bandwidthMachineToSwitch);
	for(uint32_t i = 0;i<clientNodes.GetN();++i)
		m_root.AddClientNode(clientNodes.Get (i));
	m_root.Install("ns3::RedQueue", "MinTh",GlobalProperty::m_redThreshold, "MaxTh", GlobalProperty::m_redThreshold, "QueueLimit", GlobalProperty::m_redLength);
}

// Z 建立起一个三层树形结构，叶子是机器，根和树枝是switch
void
TopologyHelper::BuildTree(NodeContainer& clientNodes)
{
	uint32_t noMachines = clientNodes.GetN();
	uint32_t noSwitches = GlobalProperty::m_noRackSwitches;
	if(noSwitches <= 1)
		NS_FATAL_ERROR("at least 2 switches in tree-topology");
	if(noMachines%noSwitches!=0)
		NS_FATAL_ERROR("cannot partition the machines uniformly");
	uint32_t noMachinesPerRack = noMachines/noSwitches;

	// [1] configure the rack switches
	for(uint32_t i=0;i<noSwitches;++i)
		m_switches.push_back(SwitchHelper());
	for(uint32_t i=0;i<noSwitches;++i){
		Names::Add ("/Names/Switch"+ToString(i), m_switches[i].GetNode());
		//m_switches[i].SetEgressQueue ("ns3::RedQueue", "MinTh",GlobalProperty::m_redThreshold, "MaxTh", GlobalProperty::m_redThreshold, "QueueLimit", GlobalProperty::m_redLength);
		m_switches[i].SetLinkDelay(StringValue ("0.04ms"));
		m_switches[i].SetDataRate(GlobalProperty::m_bandwidthMachineToSwitch);
		uint32_t low = i*noMachinesPerRack;
		uint32_t high = (i+1)*noMachinesPerRack;
		for(uint32_t j = low;j<high;++j)
			m_switches[i].AddClientNode(clientNodes.Get(j));
		m_switches[i].Install("ns3::RedQueue", "MinTh",GlobalProperty::m_redThreshold, "MaxTh", GlobalProperty::m_redThreshold, "QueueLimit", GlobalProperty::m_redLength);
	}

	// [2] configure the root switch
	Names::Add ("/Names/RootSwitch", m_root.GetNode());
	//m_root.SetEgressQueue ("ns3::RedQueue", "MinTh",GlobalProperty::m_redThreshold, "MaxTh", GlobalProperty::m_redThreshold, "QueueLimit", GlobalProperty::m_redLength);
	m_root.SetLinkDelay(StringValue ("0.04ms"));
	m_root.SetDataRate(GlobalProperty::m_bandwidthSwitchToSwitch);
	for(uint32_t i = 0;i<m_switches.size();++i)
		m_root.AddClientNode(m_switches[i].GetNode());
	m_root.Install("ns3::RedQueue", "MinTh",GlobalProperty::m_redThreshold, "MaxTh", GlobalProperty::m_redThreshold, "QueueLimit", GlobalProperty::m_redLength);
}


} /* namespace ns3 */
