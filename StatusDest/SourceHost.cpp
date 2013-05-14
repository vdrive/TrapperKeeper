// SourceHost.cpp

#include "stdafx.h"
#include "SourceHost.h"

//
//
//
SourceHost::SourceHost()
{

	Clear();
}

//
//
//
void SourceHost::Clear()
{
	CSingleLock lock(&m_data_lock,TRUE);
	m_network.clear();
	m_ip.clear();
	v_average_used_network_bandwidth.clear();
	v_average_upload_bandwidth.clear();
	v_average_download_bandwidth.clear();
	m_retreve_status_offset=0;
	m_average_used_network_bandwidth=0;
	m_average_cpu_usage=0;
	m_average_upload_bandwidth=0;
	m_average_download_bandwidth=0;
}

//
//
//
double SourceHost::GetAverageUsedNetworkBandwidth()
{
	return m_average_used_network_bandwidth;
}

//
//
//
void SourceHost::UpdateAvgUploadBandwidth(unsigned int bandwidth)
{
	CSingleLock lock(&m_data_lock,TRUE);
	if(v_average_upload_bandwidth.size() >= 20)
	{
		v_average_upload_bandwidth.erase(v_average_upload_bandwidth.begin());
	}
	v_average_upload_bandwidth.push_back(bandwidth);

	double avg = 0;
	for(UINT i =0; i<v_average_upload_bandwidth.size();i++)
	{
		avg += (double)v_average_upload_bandwidth[i];
	}
	m_average_upload_bandwidth= avg / (double)v_average_upload_bandwidth.size() /((double)(1024*1024));
}

//
//
//
double SourceHost::GetAverageUploadBandwidth()
{
	return m_average_upload_bandwidth;
}

//
//
//
void SourceHost::UpdateAvgDownloadBandwidth(unsigned int bandwidth)
{
	CSingleLock lock(&m_data_lock,TRUE);
	if(v_average_download_bandwidth.size() >= 20)
	{
		v_average_download_bandwidth.erase(v_average_download_bandwidth.begin());
	}
	v_average_download_bandwidth.push_back(bandwidth);

	double avg = 0;
	for(UINT i =0; i<v_average_download_bandwidth.size();i++)
	{
		avg += (double)v_average_download_bandwidth[i];
	}
	m_average_download_bandwidth= avg / (double)v_average_download_bandwidth.size() /((double)(1024*1024));
}

//
//
//
double SourceHost::GetAverageDownloadBandwidth()
{
	return m_average_download_bandwidth;

}

//
//
//
void SourceHost::UpdateAvgNetworkBandwidth(unsigned int bandwidth)
{
	CSingleLock lock(&m_data_lock,TRUE);
	if(v_average_used_network_bandwidth.size() >= 20)
	{
		v_average_used_network_bandwidth.erase(v_average_used_network_bandwidth.begin());
	}
	v_average_used_network_bandwidth.push_back(bandwidth);

	double avg = 0;
	for(UINT i =0; i<v_average_used_network_bandwidth.size();i++)
	{
		avg += (double)v_average_used_network_bandwidth[i];
	}
	m_average_used_network_bandwidth= avg / (double)v_average_used_network_bandwidth.size() /((double)(1024*1024));
}

//
//
//
double SourceHost::GetAverageCpuUsage()
{
	return m_average_cpu_usage;
}

//
//
//
void SourceHost::UpdateAvgCpuUsage(unsigned int usage)
{
	CSingleLock lock(&m_data_lock,TRUE);
	if(v_average_cpu_usage.size() >= 20)
	{
		v_average_cpu_usage.erase(v_average_cpu_usage.begin());
	}
	v_average_cpu_usage.push_back(usage);

	double avg = 0;
	for(UINT i =0; i<v_average_cpu_usage.size();i++)
	{
		avg += (double)v_average_cpu_usage[i];
	}
	m_average_cpu_usage= avg / (double)v_average_cpu_usage.size();
}
