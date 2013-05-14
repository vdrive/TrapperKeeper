// NoiseSockets.h

#ifndef NOISE_SOCKETS_H
#define NOISE_SOCKETS_H

#include "HttpSocket.h"
#include "NoiseModuleThreadStatusData.h"
#include <afxmt.h>	// for CCriticalSection
#include "SLSKtask.h"
#include "ProjectKeywordsVector.h"
#include "./zlib/zlib.h"

#define NOISE_DATA_LEN		(1*1024*1024)	// 1 MB
#define NOISE_BUFFER_LEN	((4*1024)+1)	// 4K + 1
//#define RAND_SEED			666

//class ConnectionData;
//class FileSharingManager;
struct spoof
{
public:
	CString m_artist;
	vector<buffer *> m_spoof;
};

class NoiseSockets
{
public:
	NoiseSockets(HWND hwnd,CCriticalSection *connection_data_critical_section);//,vector<ConnectionData> *connection_data,FileSharingManager* fsm);
	~NoiseSockets();

	DWORD ReturnNumberOfEvents();
	int ReturnNumberOfReservedEvents();
	int ReturnNumberOfSocketEvents();

	void AcceptConnection(SOCKET hSocket);

	// Public Data Members
	enum constants
	{
		NumberOfEvents=WSA_MAXIMUM_WAIT_EVENTS,
		NumberOfReservedEvents=4,
		NumberOfSocketEvents=NumberOfEvents-NumberOfReservedEvents
	};

	WSAEVENT m_events[NumberOfEvents];
	HttpSocket m_sockets[NumberOfSocketEvents];

	vector<NoiseModuleThreadStatusData> ReportStatus();

	unsigned int m_num_bogus_connections;
	unsigned int m_num_good_connections;

	CCriticalSection *p_connection_data_critical_section;
	//vector<ConnectionData> *p_connection_data;

	unsigned char *p_noise_data_buf;
	vector<SLSKtask *> tasks;
	vector<CString> names;
	vector<spoof> m_spoofs;
	void fillName(void);
	CString getSpoofName(void);
	CString GetFilePath(CString artist, CString album);
	void CreateSpoofs(void);
	int random(int lowest, int highest);
	ProjectKeywordsVector *m_projects;
	bool m_spoofs_created;
//	FileSharingManager* p_fs_manager;

private:
	HWND m_hwnd;

	//void InitNoiseData();
//	bool InitNoiseDataFromFile(char *filename);

	CTime m_time_last_initialized_noise_data;
};

#endif // NOISE_SOCKETS_H


