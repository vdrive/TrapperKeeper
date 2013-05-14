#pragma once
#include "afxcmn.h"
#include "Resource.h"
#include "RackSuperNodes.h"
#include "MyListCtrl.h"

#define WM_DONE_EDITING			WM_USER+1
#define WM_STOP_KAZAA			WM_USER_MENU+1
#define WM_RESUME_KAZAA			WM_USER_MENU+2
#define WM_RESTART_KAZAA		WM_USER_MENU+3
#define WM_REFRESH_KAZAA_LIST	WM_USER_MENU+4
#define SUB_RACK_NAME			0
//#define SUB_NUM_KAZAA			1


// CRacksSupernodesPage dialog
class SupernodeDistributerDll;
class CRacksSupernodesPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CRacksSupernodesPage)

public:
	CRacksSupernodesPage();
	virtual ~CRacksSupernodesPage();
	void InitParent(SupernodeDistributerDll* parent);
	//void UpdateRackMaxKazaaNum(RackSuperNodes& rack);


// Dialog Data
	enum { IDD = IDD_RACK_SUPERNODE_PAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNMClickRackList(NMHDR *pNMHDR, LRESULT *pResult);

public:
	CMyListCtrl m_rack_list;
	CListCtrl m_supernode_list;

private:
	SupernodeDistributerDll* p_parent;
	int m_bottom,m_border;
	void ResizePage(void);
	void GetIPStringFromInterger(int ip_int, char* ip);
	void SetCell(HWND hWnd1, CString value, int nRow, int nCol);
	CString GetItemText(HWND hWnd, int nItem, int nSubItem) const;
	void OnStopAndKillKazaa();
	void OnResumeKazaa();
	void OnRestartKazaa();
	void OnGetRemoteSupernodes();

public:
	virtual BOOL OnSetActive();
	void AddRack(RackSuperNodes& rack);
	//done editing subitems
//	LRESULT OnDoneEditing(WPARAM wparam,LPARAM lparam);
	afx_msg void OnNMRclickRackList(NMHDR *pNMHDR, LRESULT *pResult);
protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnLvnKeydownRackList(NMHDR *pNMHDR, LRESULT *pResult);
};
