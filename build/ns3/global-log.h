/*
 * global-log.h
 *
 *  Created on: Mar 28, 2017
 *      Author: tbc
 */

#ifndef SRC_MACROFLOW_UTIL_GLOBAL_LOG_H_
#define SRC_MACROFLOW_UTIL_GLOBAL_LOG_H_

#include <stdint.h>
#include <fstream>
#include <iostream>
#include <string>

#include "ns3/fatal-error.h"

namespace ns3 {

class GlobalLog {
public:
	GlobalLog();
	virtual ~GlobalLog();

	void Start(std::string logFile = "log.txt");
	void PrintLine(std::string line);
	inline void SetScreenOutput(bool screenOutput) { m_screenOutput = screenOutput; }

protected:
	std::ofstream m_out;
	uint32_t m_counter;
	bool m_screenOutput;
};

} /* namespace ns3 */

#endif /* SRC_MACROFLOW_UTIL_GLOBAL_LOG_H_ */
