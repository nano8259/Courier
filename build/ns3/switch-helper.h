/*
 * switch-helper.h
 *
 *  Created on: Mar 9, 2017
 *      Author: tbc
 */

#ifndef SRC_MACROFLOW_HELPER_SWITCH_HELPER_H_
#define SRC_MACROFLOW_HELPER_SWITCH_HELPER_H_

#include <string>
#include <vector>

#include "ns3/point-to-point-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-interface-container.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/net-device-container.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/string.h"
#include "ns3/ipv4-address.h"
#include "ns3/node.h"
#include "ns3/net-device.h"


namespace ns3 {


class SwitchHelper
{
public:
	SwitchHelper();
	virtual ~SwitchHelper() {}

	void AddClientNode(Ptr<Node> clientNode);
	void SetDataRate(const StringValue &rate);
	void SetLinkDelay(const StringValue &delay);
	Ptr<Node> GetNode() const;
	void Install(std::string type,
            std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
            std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
            std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
            std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue ());

	std::vector< Ptr<Node> > GetNodeList() const;
	std::vector< Ptr<NetDevice> > GetDeviceList() const;
	std::vector<Ipv4Address> GetIpList() const;

protected:

	void Abort(bool condition, std::string message = "") const;

	bool m_installed;
	std::string m_errorMessageInstall;
	std::string m_errorMessageNotInstall;

	NodeContainer m_selfNode;  // this switch node
	NodeContainer m_clientNodes;  // connected client nodes

	ObjectFactory m_queueFactory;

	StringValue m_dataRate;
	StringValue m_linkDelay;

	std::vector<NodeContainer> m_nodeAdjacencyList;
	std::vector<NetDeviceContainer> m_deviceAdjacencyList;
	std::vector<Ipv4InterfaceContainer> m_interfaceAdjacencyList;
};


}

#endif /* SRC_MACROFLOW_HELPER_SWITCH_HELPER_H_ */
