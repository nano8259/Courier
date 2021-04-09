/*
 * global-log.cc
 *
 *  Created on: Mar 28, 2017
 *      Author: tbc
 */

#include "global-log.h"

namespace ns3 {

GlobalLog::GlobalLog() {
	m_counter = 0;
	m_screenOutput = false;
}

GlobalLog::~GlobalLog() {
	if(m_out)
		m_out.close();
}

void
GlobalLog::Start(std::string logFile)
{
	std::cout<<"Log File: "<<logFile<<std::endl;
	m_out.open(logFile.c_str());
	if(!m_out)
		NS_FATAL_ERROR("Cannot open file: "+logFile);
}

void
GlobalLog::PrintLine(std::string line)
{
	if(m_counter++ > static_cast<uint32_t>(50000000))
		NS_FATAL_ERROR("Too many outputs!");

	if(m_screenOutput)
		std::cout<<line<<std::endl;
	m_out<<line<<std::endl;
}

} /* namespace ns3 */
