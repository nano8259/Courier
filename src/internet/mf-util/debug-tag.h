/*
 * debug-tag.h
 *
 *  Created on: Sep 6, 2017
 *      Author: tbc
 */

#ifndef SRC_INTERNET_MF_UTIL_DEBUG_TAG_H_
#define SRC_INTERNET_MF_UTIL_DEBUG_TAG_H_

#include "ns3/tag-buffer.h"
#include "ns3/tag.h"
#include "ns3/global-property.h"
#include <cstring>

namespace ns3 {

class DebugTag: public Tag {

private:
	static bool tag[1000000];
	static uint32_t counter;

	int index;

	static TypeId tid;

public:

	DebugTag(bool addCounter=false);
	virtual ~DebugTag();

	static TypeId GetTypeId (void);
	virtual TypeId GetInstanceTypeId (void) const;
	virtual uint32_t GetSerializedSize (void) const;
	virtual void Serialize (TagBuffer i) const;
	virtual void Deserialize (TagBuffer i);
	virtual void Print (std::ostream &os) const;

	void set();
	static bool check(std::ostream &os);
	int getIndex() const;

};

} /* namespace ns3 */

#endif /* SRC_INTERNET_MF_UTIL_DEBUG_TAG_H_ */
