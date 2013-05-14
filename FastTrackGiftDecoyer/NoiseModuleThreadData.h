// NoiseModuleThreadData.h

#ifndef NOISE_MODULE_THREAD_DATA_H
#define NOISE_MODULE_THREAD_DATA_H

#include "NoiseModuleThreadStatusData.h"

class NoiseModuleThreadData
{
public:
	NoiseModuleThreadData();

	vector<SOCKET> *p_incoming_connection_socket_handles;
	WSAEVENT m_reserved_events[4];
	vector<NoiseModuleThreadStatusData> *p_status_data;
	bool m_sharing_enabled;


};

#endif // NOISE_MODULE_THREAD_DATA_H