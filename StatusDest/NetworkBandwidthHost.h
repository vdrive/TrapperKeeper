// NetworkBandwidthHost.h

#ifndef NETWORK_BANDWIDTH_HOST_H
#define NETWORK_BANDWIDTH_HOST_H

class NetworkBandwidthHost
{
public:
	NetworkBandwidthHost();
	void Clear();
	bool IsUp();
	
	string m_ip;
	unsigned int m_bandwidth;
	unsigned int m_upload_bandwidth;
	unsigned int m_download_bandwidth;
	unsigned int m_cpu_usage;
	int m_files_shared;
	CTime m_last_reply_time;
};

#endif // NETWORK_BANDWIDTH_HOST_H