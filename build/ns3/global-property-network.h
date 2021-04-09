/*
 * GlobalPropertyNetwork.h
 *
 *  Created on: 2018年7月12日
 *      Author: kathy
 */

#include<stdint.h>

#ifndef SRC_NETWORK_UTILS_GLOBAL_PROPERTY_NETWORK_H_
#define SRC_NETWORK_UTILS_GLOBAL_PROPERTY_NETWORK_H_

namespace ns3 {

class GlobalPropertyNetwork {
public:
	GlobalPropertyNetwork(){}
	virtual ~GlobalPropertyNetwork(){}
	static uint64_t m_noPacketLoss[3];
	static uint64_t m_noReceived;
};

} /* namespace ns3 */

#endif /* SRC_NETWORK_UTILS_GLOBAL_PROPERTY_NETWORK_H_ */
