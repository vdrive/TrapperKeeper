#pragma once
#include "afxcmn.h"
#include "Resource.h"
#include "DCStatus.h"


// CBrowseProjectsDlg dialog

class CBrowseProjectsDlg : public CDialog
{
	DECLARE_DYNAMIC(CBrowseProjectsDlg)

public:
	CBrowseProjectsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CBrowseProjectsDlg();
	void BuildProjectTree(const char* network, vector<DCStatus>& dc_status);

// Dialog Data
	enum { IDD = IDD_BROWSE_PROJECTS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CTreeCtrl m_project_tree;
public:
	virtual BOOL OnInitDialog();
};
