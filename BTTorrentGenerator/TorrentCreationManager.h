#pragma once
#include <afxmt.h>
#include <vector>
using namespace std;

// CTorrentCreationManager

class BTTorrentGeneratorDll;
class CTorrentCreationManager 
	: private CWnd
{
	DECLARE_DYNAMIC(CTorrentCreationManager)

// functions
public:
	CTorrentCreationManager();
	virtual ~CTorrentCreationManager();

private:
	afx_msg LRESULT InitThreadData(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT DisplayTorrents(WPARAM wparam, LPARAM lparam);

	afx_msg LRESULT ResetProgressBar(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT UpdateProgressBar(WPARAM wparam, LPARAM lparam);

	afx_msg LRESULT UpdateProgressText(WPARAM wparam, LPARAM lparam);

	afx_msg void OnTimer(UINT_PTR nIDEvent);

public:
	void Init(BTTorrentGeneratorDll *pParent);
	void Run(void);
	void Stop(void);

	DECLARE_MESSAGE_MAP()

// variables
private:
	BTTorrentGeneratorDll *			m_pParent;
	CWinThread *					m_pThread;
	CCriticalSection *				m_pCriticalSection;
	HANDLE							m_aEvents[2];

	UINT_PTR						m_pingTimer;
};
