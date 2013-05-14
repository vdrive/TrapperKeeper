// CSupernodeDistributerDlg dialog
#pragma once
#include "resource.h"
#include "afxcmn.h"
#include "IPAddress.h"
#include "IPAndPort.h"
#include "afxwin.h"
#include "DlgResizer.h"


#define SUB_IP			0
#define SUB_PORT		1
//#define	SUB_STATUS		2
#define	SUB_USERNAME	2
#define SUB_UP_TIME		3
//#define SUB_DOWN_TIME	5
#define SUB_RACK		4

class HttpSocket;
class SupernodeDistributerDll;
class CSupernodeDistributerDlg : public CPropertyPage
{
	//DECLARE_DYNCREATE(CSupernodeDistributerDlg)
	DECLARE_DYNAMIC(CSupernodeDistributerDlg)

public:
	CSupernodeDistributerDlg();   // standard constructor
	virtual ~CSupernodeDistributerDlg();
	void InitParent(SupernodeDistributerDll* parent);
	void ReportStatus(HttpSocket* socket, char* ip,int& port,int& error,string& rack_ip,CTime& up_time, CTime& down_time,char *username=NULL,char *supernode_ip=NULL,char *supernode_port=NULL);
	void AddSuperNode(char* ip, char* port);
	void AddSuperNode(IPAddress& ip_address, char* kazaa_user, char* rack_name);
	void SetConnectingStatus(int ip);
	void UpdateProbingIndexStatus(UINT& probing_index, int queue_size);
	void RefreshSupernodeList(vector<IPAddress>& ip_list);
	void RefreshSupernodeList(IPAddress& ip_address);
	void RemoveSuperNodes(vector<IPAddress>& remove_list);
	void RemoveSuperNodes(vector<IPAndPort>& remove_list);
	void RemoveSuperNode(IPAndPort& remove_list);
	void RefreshSuperNodeListWithRack(char* rack_name, IPAndPort& new_ip);
	void RefreshSuperNodeListWithRack(char* rack_name, vector<IPAndPort>& new_ips);
	void RefreshSuperNodeListWithRack(char* rack_name, int ip);


// Dialog Data
	enum { IDD = IDD_SUPERNODE_DISTRIBUTER_DIALOG };
	int m_launching_interval;
private:
	int m_bottom;
	int m_border;
	UINT m_num_supernode_alive;
	UINT m_num_supernodes;
	bool m_enable_launch;
	bool m_disable_probing;
	SupernodeDistributerDll* p_parent;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedStopButton();
	afx_msg void OnBnClickedDisableButton();
	afx_msg void OnBnClickedSaveIpButton();
	afx_msg void OnBnClickedLoadIpButton2();
	afx_msg void OnBnClickedSaveUsernameButton();
	CListCtrl m_ip_list;
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDeltaposSpin2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnColumnclickIpList(NMHDR *pNMHDR, LRESULT *pResult);
private:
//	void ResizePage(void);
public:
//	virtual BOOL OnSetActive();
private:
	CString m_share_folder;
	CDlgResizer m_DlgResizer;
public:
	afx_msg void OnBnClickedAddFolderButton();
	CListBox m_share_folder_list;
	vector<CString> GetSharedFolderList(void);
private:
	void ReadFolderListFromRegistry(void);
	void SaveFolderListToRegistry(void);
	void SaveSupernodesResetHoursToRegistry(void);
	void ReadSupernodesResetHoursFromRegistry(void);

public:
	afx_msg void OnBnClickedRemoveButton();
	int m_reset_supernodes_hours;
	afx_msg void OnBnClickedApplyButton();
};