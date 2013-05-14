#pragma once
#include <set>
using namespace std;

// CDatabaseManager

class DatabaseManagerThreadData;
class CCriticalSection;
class BTDecoyClientDll;
class CDatabaseManager
	: private CWnd
{
public:
	CDatabaseManager();
	virtual ~CDatabaseManager();

	void Run(void);
	void Stop(void);

	inline void InitParent(BTDecoyClientDll *pParent) { m_pParent = pParent; }
	inline void InitVersion(unsigned int nVer) { m_nVer = nVer; }
	inline void InitCompleteStatus(bool bComplete) { m_bComplete = bComplete; }

private:
	void GetTorrents(void);

	afx_msg LRESULT InitThreadData(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT ReceivedNewTorrents(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT LogMsg(WPARAM wparam, LPARAM lparam);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()

private:
	BTDecoyClientDll *				m_pParent;
	CCriticalSection *				m_pCriticalSection;
	DatabaseManagerThreadData *		m_pThreadData;
	CWinThread *					m_pThread;

	UINT_PTR						m_getTorrentsTimer;
	unsigned int					m_nVer;
	bool							m_bComplete;
};
