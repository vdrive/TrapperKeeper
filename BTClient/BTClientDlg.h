#pragma once

#include "resource.h"
#include "ClientData.h"
#include "ModuleData.h"
#include "afxcmn.h"

// BTClientDlg dialog
class BTClientDll;
class BTClientDlg : public CDialog
{
	DECLARE_DYNAMIC(BTClientDlg)

public:
	BTClientDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~BTClientDlg();
	void DisplayInfo(char * info);

	void InitParent(BTClientDll * parent);

// Dialog Data
	enum { IDD = IDD_BTCLIENT_DLG };
	CListCtrl m_list_control;
	CListCtrl m_data_list_control;
	CListCtrl m_sent_list_control;

	void OnTimer(UINT nIDEvent);

	void DisplayClientData(vector<ClientData> cdata);
	void DisplayModuleData(vector<ModuleData> mdata);

	static int CALLBACK CompareNameProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	
	DECLARE_MESSAGE_MAP();

private:
	BTClientDll * p_parent;
public:
	afx_msg void OnLvnColumnclickDataList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnColumnclickSentList(NMHDR *pNMHDR, LRESULT *pResult);
	CTreeCtrl m_module_tree_control;
};
