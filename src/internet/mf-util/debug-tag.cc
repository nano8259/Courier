/*
 * debug-tag.cc
 *
 *  Created on: Sep 6, 2017
 *      Author: tbc
 */

#include "debug-tag.h"

namespace ns3 {

// Z TODO 没看懂tag是干啥的，这个代码中好像也没用到
bool DebugTag::tag[1000000] = {false};
uint32_t DebugTag::counter = 0;
TypeId DebugTag::tid= TypeId ("ns3::DebugTag")
			.SetParent<Tag> ();

DebugTag::DebugTag(bool addCounter) {
	index = -1;
	if(addCounter){
		index = counter++;
		tag[index] = false;
	}
}

DebugTag::~DebugTag() { }

TypeId DebugTag::GetTypeId (void){
	//static TypeId tid = TypeId ("ns3::DebugTag")
	//.SetParent<Tag> ();
	return tid;
}

TypeId DebugTag::GetInstanceTypeId (void) const {
	return GetTypeId();
}

uint32_t DebugTag::GetSerializedSize (void) const{
	return 4;
}

void DebugTag::Serialize (TagBuffer i) const{
	i.WriteU32(index);
}

void DebugTag::Deserialize (TagBuffer i){
	index = i.ReadU32();
}
void DebugTag::Print (std::ostream &os) const {
	os<<index;
}

void DebugTag::set() {
	if(index>=0){
		tag[index] = true;
		std::cout<<"set "<<index<<std::endl;
	}
}


bool DebugTag::check(std::ostream &os) {
	bool flag = true;
	for(uint32_t i=0;i<counter;++i)
		if(tag[i]==false){
			os<<i<<" ";
			flag = false;
		}
	return flag;
}

int DebugTag::getIndex() const {
	return index;
}

} /* namespace ns3 */
