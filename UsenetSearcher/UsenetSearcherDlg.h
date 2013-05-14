#pragma once

#include "resource.h"
#include "afxwin.h"
#include "ThreadData.h"

#define WM_SCAN_THREAD		WM_USER+1
#define WM_THREAD_LOG		WM_USER+2

// UsenetSearcherDlg dialog
class UsenetSearcherDll;

class UsenetSearcherDlg : public CDialog
{
	DECLARE_DYNAMIC(UsenetSearcherDlg)

public:
	UsenetSearcherDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~UsenetSearcherDlg();

	void Log(char *buf);
	void ThreadLog(char *buf);

	LRESULT ThreadLogMsg(WPARAM wparam, LPARAM lparam);
	LRESULT ScanThread(WPARAM wparam,LPARAM lparam);

//	void SendToPoster(vector<Header> v_headers, UsenetSearcherDll *dll);
	vector<Header> ExtrapolateNewPosts(vector<Header> v_headers);

	void InitParent(UsenetSearcherDll *parent);
	UsenetSearcherDll *p_dll;

// Dialog Data
	enum { IDD = IDD_USENET_SEARCHER_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT nIDEvent);
	CListBox m_list_box;
	CListBox m_thread_box;
};
