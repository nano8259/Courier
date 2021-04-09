/*
 * switch-helper.cc
 *
 *  Created on: Mar 9, 2017
 *      Author: tbc
 */


#include "switch-helper.h"

namespace ns3 {

SwitchHelper::SwitchHelper ()
{
	m_installed = false;
	m_errorMessageInstall = "Topology modification is not allowed after installing";
	m_errorMessageNotInstall = "Channels have not been installed yet";
	m_queueFactory.SetTypeId ("ns3::RedQueue");
	m_selfNode.Create(1);
	InternetStackHelper().Install(m_selfNode); // ipv4

	m_dataRate = StringValue("1000Mbps");
	m_linkDelay = StringValue("0.04ms");
}

void
SwitchHelper::AddClientNode(Ptr<Node> clientNode)
{
	Abort (m_installed, m_errorMessageInstall);
	m_clientNodes.Add(clientNode);
}


void
SwitchHelper::SetDataRate(const StringValue &rate)
{
	Abort (m_installed, m_errorMessageInstall);
	m_dataRate = rate;
}

void
SwitchHelper::SetLinkDelay(const StringValue &delay)
{
	Abort (m_installed, m_errorMessageInstall);
	m_linkDelay = delay;
}

Ptr<Node>
SwitchHelper::GetNode() const
{
	return m_selfNode.Get(0);
}

// Z 参数是Queue的参数
void
SwitchHelper::Install(std::string type,
        std::string n1, const AttributeValue &v1,
        std::string n2, const AttributeValue &v2,
        std::string n3, const AttributeValue &v3,
        std::string n4, const AttributeValue &v4)
{
	static uint32_t net = 0;
	++net;

	m_installed = true;
	PointToPointHelper p2pChannel;
	p2pChannel.SetDeviceAttribute ("DataRate", m_dataRate);
	p2pChannel.SetQueue(type,n1,v1,n2,v2,n3,v3,n4,v4);
	p2pChannel.SetChannelAttribute ("Delay", m_linkDelay);

	for(uint32_t i=0;i<m_clientNodes.GetN ();++i)
	{
		Ptr<Node> selfNode = GetNode ();
		Ptr<Node> clientNode = m_clientNodes.Get (i);
		// Z 这两个数据结构的含义是存储邻接边
		m_nodeAdjacencyList.push_back (
				NodeContainer (selfNode, clientNode));
		m_deviceAdjacencyList.push_back (
				p2pChannel.Install (selfNode, clientNode));

		Ipv4AddressHelper ipv4;
		std::ostringstream subnet;
		//modified by lkx to support more host
		// 这样就可以分配255*255个ip了
		net=i/255+1;
		subnet<<"10."<<net<<"."<<(i)%255+1<<".0";
		ipv4.SetBase (subnet.str ().c_str (), "255.255.255.0");
		// Z 为第i个，也就是刚刚添加进来的两个设备分配接口
		m_interfaceAdjacencyList.push_back(
				ipv4.Assign (m_deviceAdjacencyList[i]));
		/*subnet<<"10."<<net<<"."<<i+1<<".0";
		ipv4.SetBase (subnet.str ().c_str (), "255.255.255.0");
		m_interfaceAdjacencyList.push_back(
				ipv4.Assign (m_deviceAdjacencyList[i]));*/

	}
	//Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
}


std::vector< Ptr<Node> >
SwitchHelper::GetNodeList() const
{
	Abort (!m_installed, m_errorMessageNotInstall);
	std::vector< Ptr<Node> > nodeList;
	for(uint32_t i=0;i<m_nodeAdjacencyList.size();++i)
		nodeList.push_back(m_nodeAdjacencyList[i].Get(1));
	return nodeList;
}

std::vector< Ptr<NetDevice> >
SwitchHelper::GetDeviceList() const
{
	Abort (!m_installed, m_errorMessageNotInstall);
	std::vector< Ptr<NetDevice> > deviceList;
	for(uint32_t i=0;i<m_deviceAdjacencyList.size();++i)
		deviceList.push_back(m_deviceAdjacencyList[i].Get(1));
	return deviceList;
}

std::vector<Ipv4Address>
SwitchHelper::GetIpList() const
{
	Abort (!m_installed, m_errorMessageNotInstall);
	std::vector<Ipv4Address> ipList;
	for(uint32_t i=0;i<m_interfaceAdjacencyList.size();++i)
		ipList.push_back(m_interfaceAdjacencyList[i].GetAddress(1));
	return ipList;
}


void
SwitchHelper::Abort(bool condition, std::string message) const
{
	if(condition)
		NS_FATAL_ERROR (message);
}


}

