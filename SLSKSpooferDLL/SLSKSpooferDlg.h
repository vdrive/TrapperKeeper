// SLSKSpooferDlg.h : header file
//

#pragma once
#include "Resource.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "SLSKController.h"
//#include "SLSKaccountInfo.h"
#include "SLSKToDoList.h"
#include "ProjectManager.h"

struct account
{
public:
	CString un, pass;
};

 //CSLSKSpooferDlg dialog
//class SLSKController;
class CSLSKSpooferDlg : public CDialog
{
// Construction
public:
	CSLSKSpooferDlg(CWnd* pParent = NULL);	// standard constructor

	SLSKController m_controller;
	ProjectManager m_project_manager;
	//vector<TrackInformation*> m_track_info;
	ProjectKeywordsVector m_projects;
	bool m_projects_changed;

	unsigned int m_total_num_received_connections;
	unsigned int m_num_good_connections;
	unsigned int m_num_bogus_connections;
	bool m_started_spoofing;
	void FillProjectsListView(void);
//	SLSKaccountInfo account_info;
	SLSKToDoList todo;
	int m_modnumber;

// Dialog Data
	enum { IDD = IDD_SLSKSPOOFER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	
	

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CEdit un_edit_box;
	CEdit pass_edit_box;
	CEdit port_edit_box;
	afx_msg void OnBnClickedButton1();
	CRichEditCtrl status_edit_box;
	afx_msg void OnBnClickedButton2();
	afx_msg void OnEnChangeRichedit22();
	afx_msg void OnTimer(UINT nIDEvent);
	vector<account> accounts;
	void Log(const char* log);
	void LogProjects(const char* log);
	void setServerStatus(const char* status);
	void setParentStatus(const char* status);
	void setPeerStatus(const char* status);
	void setPeerConnectingStatus(const char* status);
	void setParentConnectingStatus(const char* status);
	void setPeerModStatus(const char* status);
	void setParentModStatus(const char* status);
	void setServerModStatus(const char* status);
	int connectioncount;
private:
	//SLSKController *p_parent;
public:
	CEdit noc;
	CListBox m_log_list;
	CEdit m_server_status;
	CEdit m_peer_status;
	CEdit m_parent_status;
//	void InitParent(SLSKController* parent);
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedUpdate();
	CListBox m_project_log;
	CEdit m_num_parents_connecting;
	CEdit m_num_peer_connecting;
	CEdit m_num_server_mods;
	CEdit m_num_parent_mods;
	CEdit m_num_peer_mods;
	afx_msg void OnLbnSelchangeList1();
};
