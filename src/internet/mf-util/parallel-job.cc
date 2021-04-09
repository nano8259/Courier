/*
 * parallel-job.cc
 *
 *  Created on: Mar 21, 2017
 *      Author: tbc
 */

#include "parallel-job.h"



namespace ns3 {

ParallelJob::ParallelJob() {
	m_initialzed = false;
}

void
ParallelJob::Initialize(uint32_t jobId, uint8_t jobType)
{
	if(m_initialzed)
		NS_FATAL_ERROR ("ParallelJob has been initialized!");
	m_initialzed = true;
	uint8_t checker = jobType & static_cast<uint8_t>(0xff ^ 0x3);
	if(checker)
		NS_FATAL_ERROR ("illegal jobType!");

	m_jobType = jobType;
	m_jobId = jobId;

	m_state = Pending;

	m_noMappers = 0;
	m_noReducers = 0;
	m_totalBytes = 0;
	m_ciMachine = static_cast<uint32_t>(-1);

    static bool init = false;
    if(!init){
    	init=true;
    	srand(GlobalProperty::m_seed);
    }
}

void
ParallelJob::InitializeCiProperty(Time ciDuration, Time arrivalTime)
{
	if(!m_initialzed)
		NS_FATAL_ERROR ("ParallelJob is not initialized!");

	m_ciDuration = ciDuration;
	m_arrivalTime = arrivalTime;
}

void
ParallelJob::InitializeNiProperty(uint32_t noMappers, uint32_t noReducers, Time arrivalTime, std::vector<uint32_t> originalMapperMachine)
{
	if(!m_initialzed)
		NS_FATAL_ERROR ("ParallelJob is not initialized!");

	m_noMappers = noMappers;
	m_noReducers = noReducers;
	m_arrivalTime = arrivalTime;
	
    static uint32_t jobId = 0;
    std::stringstream ss;
    ss<<jobId++<<": ";
    for(uint32_t i=0;i<m_noMappers;++i){
		// Z allocate mappers' location
		if(GlobalProperty::m_randomMapping) m_mapperMachines.push_back(rand() % (GlobalProperty::m_noMachines*143)/143);
		else m_mapperMachines.push_back(originalMapperMachine[i] % GlobalProperty::m_noMachines);
        ss<<m_mapperMachines[i]<<" ";
    }
    //Log0(ss.str()); //lkx

	m_noActiveMfs = 0;
	m_noEmittedMfs = 0;
	m_noFinishedMfs = 0;
}

void ParallelJob::AddMacroflow(Ptr<Macroflow> mf)
{
	if(!m_initialzed)
		NS_FATAL_ERROR ("ParallelJob is not initialized!");

	m_macroflowList.push_back(mf);
	m_totalBytes += mf->GetBytes();
}



} /* namespace ns3 */
