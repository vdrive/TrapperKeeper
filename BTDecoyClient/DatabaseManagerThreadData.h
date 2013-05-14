#pragma once
#include "WSocket.h"

#define WM_DBMANAGER_INIT_THREAD_DATA			WM_USER+1
#define WM_DBMANAGER_GET_TORRENTS				WM_USER+2
#define WM_DBMANAGER_RECEIVED_TORRENTS			WM_USER+3
#define WM_DBMANAGER_LOG_MSG					WM_USER+4

#define DBMANAGER_EVENTS						2

class DatabaseManagerThreadData
{
public:

	DatabaseManagerThreadData(HANDLE aEvents[])
	{
		memcpy( m_aEvents, aEvents, sizeof( HANDLE ) * DBMANAGER_EVENTS );
	}

	~DatabaseManagerThreadData(void)
	{
		memset( m_aEvents, 0, sizeof( HANDLE ) * DBMANAGER_EVENTS );
	}

public:
	HANDLE		m_aEvents[DBMANAGER_EVENTS];
};


