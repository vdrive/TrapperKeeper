// NoiseModuleThreadStatusData.h

#ifndef NOISE_MODULE_THREAD_STATUS_DATA_H
#define NOISE_MODULE_THREAD_STATUS_DATA_H

class NoiseModuleThreadStatusData
{
public:
	NoiseModuleThreadStatusData();
	void Clear();

	bool m_is_idle;

	CString m_filename;
	unsigned int m_start;
	unsigned int m_end;
	unsigned int m_num_sent;
	unsigned int m_total_size;

	unsigned short int m_mod;
	unsigned short int m_socket;

//	string m_remote_ip;
	unsigned int m_remote_ip;

	CTime m_start_time;

	CString m_client;

	unsigned int m_num_bogus_connections;
	unsigned int m_num_good_connections;
};

#endif // NOISE_MODULE_THREAD_STATUS_DATA_H