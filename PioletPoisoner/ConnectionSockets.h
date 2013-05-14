// ConnectionSockets.h
#pragma once

#include "PioletFTSocket.h"
//#include "ConnectionModuleStatusData.h"
#define NOISE_DATA_LEN		(1*1024*1024)	// 1 MB
#define NOISE_BUFFER_LEN	((4*1024)+1)	// 4K + 1


class ConnectionSockets
{
public:
	// Public Member Functions
	ConnectionSockets();
	~ConnectionSockets();

	DWORD ReturnNumberOfEvents();
	int ReturnNumberOfReservedEvents();
	int ReturnNumberOfSocketEvents();
	//void ConnectToHost(char* ip, int port, UINT file_length, const char* md5);
	void AcceptConnection(SOCKET hSocket,char* ip, int port, UINT file_length, const char* md5);
	void CloseIdleSockets();

//	ConnectionModuleStatusData ReportStatus();

	// Public Data Members
	enum constants
	{
		NumberOfEvents=WSA_MAXIMUM_WAIT_EVENTS,
		NumberOfReservedEvents=4,
		NumberOfSocketEvents=NumberOfEvents-NumberOfReservedEvents
	};

	int GetNumIdleSockets();
	WSAEVENT m_events[NumberOfEvents];
	PioletFTSocket m_sockets[NumberOfSocketEvents];
//	ConnectionModuleStatusData m_status_data;

	unsigned char *p_noise_data_buf;

private:
	void InitNoiseData();
	bool InitNoiseDataFromFile(char *filename);
	CTime m_time_last_initialized_noise_data;

};