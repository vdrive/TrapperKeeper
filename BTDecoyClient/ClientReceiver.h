#pragma once
#include "ListeningSocket.h"

class IP;
class BTDecoyClientDll;
class CCriticalSection;
class ClientReceiverThreadData;
class CClientReceiver
	: private CWnd
{
// functions
public:
	CClientReceiver(void);
	~CClientReceiver(void);

	inline void InitParent(BTDecoyClientDll *pParent) { m_pParent = pParent; }
	void Run(void);
	void Stop(void);

	void SetOpen(void);
	void SetClosed(void);

	inline bool IsOpen(void) { return m_eStatus == ListeningSocket::ListenerStatus::OPEN; }
	inline bool IsClosed(void) { return m_eStatus == ListeningSocket::ListenerStatus::CLOSED; }

private:
	afx_msg LRESULT InitThreadData(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT NewConnection(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT CurrentStatus(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT LogMsg(WPARAM wparam, LPARAM lparam);

	DECLARE_MESSAGE_MAP()

// variables
private:
	BTDecoyClientDll *						m_pParent;
	CCriticalSection *						m_pCriticalSection;
	ClientReceiverThreadData *				m_pThreadData;
	CWinThread *							m_pThread;

	ListeningSocket::ListenerStatus			m_eStatus;
};
