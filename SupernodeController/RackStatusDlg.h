#pragma once
#include "afxcmn.h"
#include "Resource.h"


// CRackStatusDlg dialog

class SupernodeControllerDll;
class CRackStatusDlg : public CDialog
{
	DECLARE_DYNAMIC(CRackStatusDlg)

public:
	CRackStatusDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRackStatusDlg();
	void InitParent(SupernodeControllerDll* parent);
	void AddRack(const char * rack);


// Dialog Data
	enum { IDD = IDD_RACK_STATUS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_rack_list;
	CListCtrl m_supernode_list;

private:
	SupernodeControllerDll* p_parent;
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnNMClickRackList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnKeydownRackList(NMHDR *pNMHDR, LRESULT *pResult);
};
