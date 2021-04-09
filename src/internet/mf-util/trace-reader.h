/*
 * trace-reader.h
 *
 *  Created on: Mar 21, 2017
 *      Author: tbc
 */

#ifndef SRC_MACROFLOW_UTIL_TRACE_READER_H_
#define SRC_MACROFLOW_UTIL_TRACE_READER_H_

#include <vector>
#include <fstream>
#include <sstream>
#include <string>

#include "ns3/ptr.h"
#include "ns3/nstime.h"

#include "ns3/global-property.h"
#include "ns3/tracker.h"

#include "parallel-job.h"
#include "macroflow.h"

namespace ns3 {

class TraceReader {
public:
	TraceReader(std::string traceFile);
	virtual ~TraceReader() {}

	std::vector<Ptr<ParallelJob> > Analysis();
	inline uint64_t GetTotalBytes() const { return m_totalBytes; }


protected:
	std::vector<std::string> Split(std::string str) const;
	uint32_t StringToInteger(std::string str) const;

	std::string m_traceFile;
	uint64_t m_totalBytes;

private:
	// static inline uint32_t makepair(uint32_t jobId, uint32_t reducerId) { return (jobId<<16) + reducerId; }
};

} /* namespace ns3 */

#endif /* SRC_MACROFLOW_UTIL_TRACE_READER_H_ */
