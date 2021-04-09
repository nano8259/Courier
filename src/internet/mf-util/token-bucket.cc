/*
 * token-bucket.cc
 *
 *  Created on: Sep 27, 2017
 *      Author: tbc
 */

#include "token-bucket.h"

namespace ns3 {

TokenBucket::TokenBucket(uint32_t max_bytes) {
	this->max_bytes = max_bytes;
	cur_bytes = 0;
}

TokenBucket::~TokenBucket() { }

} /* namespace ns3 */
