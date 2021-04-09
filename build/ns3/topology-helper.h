/*
 * topology-helper.h
 *
 *  Created on: Apr 7, 2017
 *      Author: tbc
 */

#ifndef SRC_MACROFLOW_HELPER_TOPOLOGY_HELPER_H_
#define SRC_MACROFLOW_HELPER_TOPOLOGY_HELPER_H_

#include <vector>
#include "ns3/switch-helper.h"
#include "ns3/global-property.h"
#include "ns3/names.h"
#include "ns3/fatal-error.h"
#include "ns3/ptr.h"
#include "ns3/node.h"
#include "ns3/ipv4-global-routing-helper.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <iomanip>
#include <boost/format.hpp>

namespace ns3 {

class TopologyHelper {
public:
	TopologyHelper();
	virtual ~TopologyHelper();
	Ptr<Node> Build(NodeContainer& clientNodes);
	std::vector< Ptr<Node> > GetNodeList() const;
	std::vector<Ipv4Address> GetIpList() const;


private:

	void BuildSingle(NodeContainer& clientNodes);
	void BuildTree(NodeContainer& clientNodes);
	void BuildDropTail(NodeContainer& clientNodes);


	SwitchHelper m_root;
	std::vector<SwitchHelper> m_switches;
};

} /* namespace ns3 */

#endif /* SRC_MACROFLOW_HELPER_TOPOLOGY_HELPER_H_ */
