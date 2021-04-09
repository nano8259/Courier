/*
 * mf-trace-reader.cc
 *
 *  Created on: Mar 21, 2017
 *      Author: tbc
 */

// Z 本代码的作用是读取facebook的trace作为benchmark

#include "trace-reader.h"


namespace ns3 {

// Z 这个flie设定为full.txt，可能就是数据集吧
TraceReader::TraceReader(std::string traceFile) {
	m_traceFile = traceFile;
	m_totalBytes = 0;
}


std::vector<Ptr<ParallelJob> >
TraceReader::Analysis()
{
	std::ifstream fin(m_traceFile.c_str());
	if(!fin)
		NS_FATAL_ERROR ("Cannot open file: " + m_traceFile);

	std::stringstream ss;

	std::string line;
	std::getline(fin, line);
	ss.str(line);

	uint32_t noJobs;
	ss >> noJobs; // number of machines in traces, not used
	ss >> noJobs; // number of jobs
	GlobalProperty::m_noTotalNiJobs = noJobs;

	Macroflow::ReportMapperList = &Tracker::ReportMapperList;
	Macroflow::GetCandidateList = &Tracker::GetCandidateList;
	Macroflow::ReportChosenHosts = &Tracker::ReportChosenHosts;
	Macroflow::ReportPausedHosts = &Tracker::ReportPausedHosts;

	//uint64_t dbg_totalsize = 0;
	std::vector<Ptr<ParallelJob> > jobs;
	// Z TODO 如果m_niDuration大于0，则赋值为11B。。啥意思  但是项目中是0
	uint8_t jobType = (GlobalProperty::m_niDuration.GetMilliSeconds()>0) ? (ParallelJob::NiJob | ParallelJob::CiJob) : (ParallelJob::NiJob);
	while(noJobs--){
		std::getline(fin, line);
		std::vector<std::string> elem = Split(line);
		uint32_t index = 0;

		uint32_t jobId = StringToInteger(elem[index++])-1;
		uint32_t arrivalTime = StringToInteger(elem[index++]);
		// Z time-scale = 0.0005
		// Z TODO 为啥要对到达时间进行放缩？为了减少模拟时间吗
		arrivalTime = static_cast<uint32_t>(arrivalTime*GlobalProperty::m_timeScale);

		uint32_t noMappers = StringToInteger(elem[index++]);
		std::vector<uint32_t> mapperMachines;
		for(uint32_t i=0;i<noMappers;++i)
			mapperMachines.push_back(StringToInteger(elem[index++]));

		uint32_t noReducers = StringToInteger(elem[index++]);

		Ptr<ParallelJob> job = Create<ParallelJob>();
		job->Initialize(jobId, jobType);
		// Z 所以这个意思是：一个job可以既是ci，也是ni？
		job->InitializeNiProperty(noMappers, noReducers, MilliSeconds(arrivalTime), mapperMachines);
		if((jobType&ParallelJob::CiJob)!=0)
			job->SetCiDuration(GlobalProperty::m_niDuration);

		for(uint32_t i=0;i<noReducers;++i){
			std::string reducerPair = elem[index++];
			reducerPair = reducerPair.replace(reducerPair.find(":"),1," ");
			std::vector<std::string> splitPair = Split(reducerPair);
			uint64_t macroflowBytes = StringToInteger(splitPair[1]);
			macroflowBytes = macroflowBytes * 1024 * 1024; // from MB to Byte
			// Z 项目中m_sizeScale为0.00015
			macroflowBytes = static_cast<uint64_t>(macroflowBytes * GlobalProperty::m_sizeScale);
			// Z 把所有的bytes加起来
			m_totalBytes += macroflowBytes;
			// Z 是否大于4GB
	        if( macroflowBytes/noMappers >= (1ULL<<32) )// temp disable by tbc
				NS_FATAL_ERROR("Too large flow size in job " + ToString(jobId) + " mf " + ToString(i));

			Ptr<Macroflow> mf = Create<Macroflow>();
			// Z 每个reducer起一个mf
			mf->Initialize(makepair(jobId, i),job,macroflowBytes);
			mf->SetMapperList(job->GetMapperMachineList()); // 2017-08-26 to support fetcher
			job->AddMacroflow(mf);
		}
		jobs.push_back(job);
	}

	GlobalProperty::m_TotalBytes = m_totalBytes;
	fin.close();
	return jobs;
}

// Z 把一个字符串分割成vector?
// Z 分割的标准是>>的标准，没细看，应该是空格？
std::vector<std::string>
TraceReader::Split(std::string str) const
{
	std::stringstream ss;
	ss.str(str);
	std::vector<std::string> vec;
	std::string temp;
	while(ss >> temp)
		vec.push_back(temp);
	return vec;
}

// Z 字符串转整形
uint32_t
TraceReader::StringToInteger(std::string str) const
{
	uint32_t x = 0;
	for(uint32_t i = 0;i<str.length();++i){
		if(!isdigit(str[i]))
			break;
		x = x*10 + (str[i]-'0');
	}

	return x;
}

} /* namespace ns3 */
