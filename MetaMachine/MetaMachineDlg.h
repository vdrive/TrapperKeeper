#pragma once
#include "resource.h"
#include "afxcmn.h"
#include "afxwin.h"
#include "MetaTCP.h"
#include "Buffer2000.h"
// CMetaMachineDlg dialog

class CMetaMachineDlg : public CDialog
{
	DECLARE_DYNAMIC(CMetaMachineDlg)

	class LogMsg : public Object{
	private:
		CString m_msg;
	public:
		LogMsg(const char *msg){
			m_msg=msg;
		}
	
		inline CString GetMsg(){return m_msg;}
	};


public:
	CMetaMachineDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMetaMachineDlg();

// Dialog Data
	enum { IDD = IDD_METAMACHINEDIALOG };

protected:
	//MetaTCP m_tcp;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	afx_msg void OnTimer(UINT nIDEvent);
public:

	CEdit m_search_string;
	CRichEditCtrl m_returns;
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
protected:
	virtual void PostNcDestroy();
public:
	afx_msg void OnClose();
protected:
	virtual void OnOK();
	virtual void OnCancel();

	//static void Log(info);

public:
	virtual BOOL DestroyWindow();
	void Log(const char* info);

	void ServerLog(const char* ip,Buffer2000 &data);
	UINT GetMyID(void);

	afx_msg void OnSave();
	CTreeCtrl m_project_info;
	afx_msg void OnProjectInfoRefresh();
	CTreeCtrl m_server_status_tree;
	afx_msg void OnRefreshServers();
	afx_msg void OnBuildSpoofTable();
	afx_msg void OnReportResults();

private:
	HANDLE m_log_file;
	HANDLE m_log_file2;
	HANDLE m_log_file3;
	Vector mv_needed_logs;
	Vector mv_needed_server_logs;
	Vector mv_failed_queries;
	int m_failed_queries;
	int m_remaining_queries;
public:
	void AddFailedQuery(const char* query);
	afx_msg void OnStnClickedFailquerycount();
	CStatic m_fail_query_count_control;
	CListBox m_lb_failed_queries;
	CStatic m_s_remaining_queries;
	void SetRemainingQueries(int queries);
private:
	CStatic m_main_thread_alive;
public:
	void WriteToLogFile(const char* str);
	void WriteToLogFile2(const char* str);
	void WriteToLogFile3(const char* str);
	afx_msg void OnUpdateProjectsDatabase();
	afx_msg void OnBuildSwarmTable();
	afx_msg void OnBuildSwarmTable2();
	CRichEditCtrl m_server_log;
	afx_msg void OnBuildFalseDecoyTable();
};
