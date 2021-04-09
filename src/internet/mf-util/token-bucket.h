/*
 * token-bucket.h
 *
 *  Created on: Sep 27, 2017
 *      Author: tbc
 */

#ifndef SRC_INTERNET_MF_UTIL_TOKEN_BUCKET_H_
#define SRC_INTERNET_MF_UTIL_TOKEN_BUCKET_H_

#include "ns3/object.h"
#include "ns3/global-property.h"


namespace ns3 {

class TokenBucket : public Object{
private:
	uint32_t max_bytes;
	uint32_t cur_bytes;
public:
	TokenBucket(uint32_t max_bytes);
	virtual ~TokenBucket();

	inline bool Get(){
		if(cur_bytes>=1500){
			cur_bytes-=1500;
			return true;
		}
		return false;
	}

	inline void UnGet(){
		cur_bytes+=1500;
		if(cur_bytes>max_bytes)
			cur_bytes = max_bytes;
	}

	inline void Add(uint32_t speed){
		cur_bytes += speed;
		if(cur_bytes>max_bytes)
			cur_bytes = max_bytes;
	}

	inline uint32_t Total() const {
		return cur_bytes;
	}
};

} /* namespace ns3 */

#endif /* SRC_INTERNET_MF_UTIL_TOKEN_BUCKET_H_ */
