#pragma once
#include "SupernodeDistributerDlg.h"
#include "RacksSupernodesPage.h"


// CMainDlg
class SupernodeDistributerDll;
class CMainDlg : public CPropertySheet
{
	DECLARE_DYNAMIC(CMainDlg)

public:
	CMainDlg(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CMainDlg(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~CMainDlg();
	void InitParent(SupernodeDistributerDll* parent);

protected:
	DECLARE_MESSAGE_MAP()
	BOOL   m_bNeedInit;
	CRect  m_rCrt;
	int    m_nMinCX;
	int    m_nMinCY;
	HICON m_hIcon;

public:
	CSupernodeDistributerDlg m_supernode_distributer_page;
	CRacksSupernodesPage m_racks_supernodes_page;

private:
	SupernodeDistributerDll* p_parent;
//	int m_bottom;
//	int m_border;
public:
	afx_msg void OnClose();
	void OnExit(void);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
};


