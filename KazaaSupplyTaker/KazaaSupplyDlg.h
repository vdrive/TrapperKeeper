// KazaaSupplyDlg.h : header file
//

#if !defined(AFX_KAZAASUPPLYDLG_H__77E5AF8B_0C1D_44C8_A633_C0085AEFE6FA__INCLUDED_)
#define AFX_KAZAASUPPLYDLG_H__77E5AF8B_0C1D_44C8_A633_C0085AEFE6FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\KazaaLauncher\KazaaLauncherInterface.h"
#include "KazaaManager.h"
#include "WorkerGuy.h"
#include "afxwin.h"

#define WM_THREAD_LOG					WM_USER+1
//#define WM_PROCESS_RAW_DATA				WM_USER+2
//#define WM_RAW_DATA_PROCESSING_COMPLETE	WM_USER+3
#define WM_RAW_DUMP						WM_USER+4
#define WM_FIND_MORE_DUMP				WM_USER+5

/////////////////////////////////////////////////////////////////////////////
// CKazaaSupplyDlg dialog

class CKazaaSupplyDlg : public CDialog
{
// Construction
public:
	CKazaaSupplyDlg(CWnd* pParent = NULL);	// standard constructor

	LRESULT ThreadLog(WPARAM wparam,LPARAM lparam);
	LRESULT ProcessRawData(WPARAM wparam,LPARAM lparam);
	LRESULT ProcessingRawDataComplete(WPARAM wparam,LPARAM lparam);
	LRESULT RawDump(WPARAM wparam, LPARAM lparam);
	LRESULT FindMoreDump(WPARAM wparam, LPARAM lparam);

	void Log(int worker_id, HWND kza_hwnd, char *buf);
	void Log(int worker_id, int start_ip, int end_ip, char *buf);

	void DoState();
	void OnCancel();
	void StopTimer();
	void StartTimer();
	void ResetWatchDogTimers();

	KazaaLauncherInterface m_kza_launcher;
	KazaaManager manager;
	vector<WorkerGuy *>::iterator m_worker_iter;

//	unsigned int m_NumKazaa;	
//	int m_big_state;
	bool m_running;
	bool m_stop_pressed;

	// Desktop Handles
	HDESK m_desk1;
	HDESK m_desk2;
	HDESK m_desk3;

	// Dialog Data
	//{{AFX_DATA(CKazaaSupplyDlg)
	enum { IDD = IDD_KAZAASUPPLYTAKER_DIALOG };
	CEdit	m_edit_box;
	CListBox	m_list_box;
	CButton	m_button_go;
//	int		m_MaxKazaa;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKazaaSupplyDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CKazaaSupplyDlg)
	virtual BOOL OnInitDialog();
//	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	afx_msg void OnBnClickedOnTopCheck();
	afx_msg void OnBnClickedMinimizeCheck();
	afx_msg void OnGo();	// public so that the dll can call it
	afx_msg void OnStop();	//   "			"			"

	CButton m_button_stop;
	
public:

	CEdit m_search_more_edit;
	CStdioFile m_log_file;

	CTime m_yesterday;
	afx_msg void OnBnClickedRequestSnodeButton();
	void CheckForErrorBoxes();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KAZAASUPPLYDLG_H__77E5AF8B_0C1D_44C8_A633_C0085AEFE6FA__INCLUDED_)
