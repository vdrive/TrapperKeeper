#pragma once
#include "ListeningSocket.h"
#include "IPPort.h"

#define CLIENTRECEIVER_RESERVED_EVENTS		2

typedef struct ClientReceiverThreadDataStruct
{
	WSAEVENT								m_aReservedEvents[CLIENTRECEIVER_RESERVED_EVENTS];
	ListeningSocket::ListenerStatus *		m_peStatus;
} ClientReceiverThreadData;

class BTTrackerDll;
class CCriticalSection;
class CClientReceiver : private CWnd
{
// functions
public:
	CClientReceiver(void);
	~CClientReceiver(void);

	void Init(BTTrackerDll *pParent);
	void Run(void);
	void Stop(void);

	void SetOpen(void);
	void SetFull(void);
	void SetClosed(void);

	inline bool IsOpen(void) { return m_eStatus == ListeningSocket::OPEN; }
	inline bool IsClosed(void) { return m_eStatus == ListeningSocket::CLOSED; }

private:
	afx_msg LRESULT InitThreadData(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT NewConnection(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT CurrentStatus(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT LogMsg(WPARAM wparam, LPARAM lparam);

	DECLARE_MESSAGE_MAP()

// variables
private:
	BTTrackerDll *							m_pParent;
	CCriticalSection *						m_pCriticalSection;
	ClientReceiverThreadData *				m_pThreadData;
	CWinThread *							m_pThread;

	ListeningSocket::ListenerStatus			m_eStatus;
};
