#pragma once
#include "Resource.h"
#include "afxwin.h"
#include "Hub.h"
#include "UDPSearchResult.h"
#include "HubListCollector.h"
#include "DCClientHubSocket.h"
#include "ProjectManager.h"
#include "DownloadManager.h"

#define SCANNERS	200
#define SPOOFS	3

// DCScannerDlg dialog

class DCSupplyDlg : public CDialog
{
	DECLARE_DYNAMIC(DCSupplyDlg)

public:
	DCSupplyDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~DCSupplyDlg();
	void StatusUpdate(char *post_data);	//displays the status of what projects are being checked
	void DBUpdate(char *post_data);	//displays activity of database
	void SetProjectManager(ProjectManager &pm);	//set one project manager to the dialog class
	int CreateRandomSocket();
	void ConnectAllHubs();
	void HubRetry();
	int SearchConnectedHubs();
	void DisconnectAll();
// Dialog Data
	enum { IDD = IDD_DC_SUPPLY_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	// List box that displays status of plug-in
	CListBox StatusListBox;
	hublistcollector m_listcollector;
	DCClientHubSocket hubsocket[SCANNERS];
	void ReceivedHubList(vector<Hub> hub_list);		//hub list downloaded from http protocol
	UDPSearchResult resultant;	//object that takes care of the searches and their respective results 
	vector<Hub> m_hub_list;	//a vector of hub objects
	vector<Hub> v_retry_hub;	//vector of all hubs to retry
	ProjectManager *p_pm;	//pointer to project manager object
	ProjectKeywordsVector *m_keywords_vector;	//object received from project manager
	DownloadManager m_download_manager;
	afx_msg void OnTimer(UINT nIDEvent);
	CEdit m_connected_hubs;
	afx_msg void OnBnClickedClose();
	// listbox that show what is being inserted into the database
	CListBox db_activity;
	// matches that will be entered into the databse
	CEdit m_matches;

	unsigned int m_num_matches;
	bool b_projects_changed;
	bool b_hub_connect;
	int m_hublist_position;	//indicates where in the hub list we have iterated through
	int m_search_position;	//indicates where in the vector we last left off
	int m_loops;	//determines how many times we have looped the hubs
	afx_msg void OnDestroy();
};
