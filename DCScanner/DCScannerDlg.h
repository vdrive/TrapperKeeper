#pragma once
#include "Resource.h"
#include "afxwin.h"
#include "Hub.h"
#include "UDPSearchResult.h"
#include "HubListCollector.h"
#include "DCClientHubSocket.h"
#include "ProjectManager.h"
#include "DownloadManager.h"
#include "SpoofCollector.h"
#include "DB.h"	//database class
//#include "ShareManager.h"
#define SCANNERS	1
#define SPOOFS	50

// DCScannerDlg dialog

class DCScannerDlg : public CDialog
{
	DECLARE_DYNAMIC(DCScannerDlg)

public:
	DCScannerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~DCScannerDlg();
	void OnCancel();
	void StatusUpdate(char *post_data);
	void SetProjectManager(ProjectManager &pm);
	void IncrementSpoofs(int increment);
	vector<SpoofCollector> GetSpoofCollector();
	void ClearSpoofCollector();		//clear spoof collector vector
	void DisconnectAll();		//reset all connections

// Dialog Data
	enum { IDD = IDD_DC_SCANNER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	// List box that displays status of plug-in
	//ProjectManager m_proj_manager;
	CListBox StatusListBox;
	hublistcollector m_listcollector;
	DCClientHubSocket hubsocket[SCANNERS];
	void ReceivedHubList(vector<Hub> hub_list);
	//UDPSearchResult resultant;
	int m_hublist_position;	//indicates where in the hub list we have iterated through
	vector<Hub> m_hub_list;
	afx_msg void OnTimer(UINT nIDEvent);
	CEdit m_connected_hubs;
	DB db_connection;
	ProjectManager *p_pm;	//pointer to project manager object
	ProjectKeywordsVector *m_keywords_vector;	//object received from project manager
	bool b_projects_changed;
	DownloadManager m_download_manager;
	afx_msg void OnDestroy();
	unsigned int m_spoofs_sent;	//int that keeps track of spoofs sent
	// display variable that keeps track of spoofs sent
	CEdit spoofs_sent_text;
	CEdit m_active_projects;
	char m_buffer[20];
	string m_date;
	Hub m_current_hub;
	int m_hubs_connected;
	afx_msg void OnBnClickedButton1();
};
