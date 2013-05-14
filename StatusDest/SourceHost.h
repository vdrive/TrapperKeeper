// SourceHost.h

#ifndef SOURCE_HOST_H
#define SOURCE_HOST_H
#include <afxmt.h>	// for CCriticalSection

#include <afxmt.h>

class SourceHost
{
private:
	CCriticalSection m_data_lock;
public:
	void UpdateAvgNetworkBandwidth(unsigned int bandwidth);
	double GetAverageUsedNetworkBandwidth();
	void UpdateAvgUploadBandwidth(unsigned int bandwidth);
	double GetAverageUploadBandwidth();
	void UpdateAvgDownloadBandwidth(unsigned int bandwidth);
	double GetAverageDownloadBandwidth();
	void UpdateAvgCpuUsage(unsigned int usage);
	double GetAverageCpuUsage();
	void operator=(const SourceHost &copy)
	{
		m_network=copy.m_network;
		m_ip=copy.m_ip;
		m_retreve_status_offset=copy.m_retreve_status_offset;
		m_average_used_network_bandwidth;
		m_average_cpu_usage;
		m_average_upload_bandwidth;
		m_average_download_bandwidth;
	};
	SourceHost(const SourceHost &copy)
	{
		*this=copy;
	};

	SourceHost();
	void Clear();

	string m_network;
	string m_ip;

	int m_retreve_status_offset;

private:
	//CCriticalSecion m_data_lock;

	double m_average_used_network_bandwidth;
	double m_average_cpu_usage;
	double m_average_upload_bandwidth;
	double m_average_download_bandwidth;

	vector<unsigned int> v_average_used_network_bandwidth;
	vector<unsigned int> v_average_cpu_usage;
	vector<unsigned int> v_average_upload_bandwidth;
	vector<unsigned int> v_average_download_bandwidth;
};

#endif // SOURCE_HOST_H