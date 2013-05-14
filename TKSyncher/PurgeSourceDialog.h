#pragma once
#include "TKComInterface.h"
#include "../SamplePlugIn/dll.h"
#include "resource.h"


// CPurgeSourceDialog dialog

class CPurgeSourceDialog : public CDialog
{
	DECLARE_DYNCREATE(CPurgeSourceDialog)

public:
	CPurgeSourceDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPurgeSourceDialog();
// Dialog Data
	enum { IDD = IDD_DELETE_SOURCE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	TKComInterface m_com;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCancel();
	afx_msg void OnPurge();
	CIPAddressCtrl m_ip_start;
	CIPAddressCtrl m_ip_end;
	CEdit m_source_name;
	void SetDLL(Dll* p_dll);
	afx_msg void OnTotalAnnihilation();
	afx_msg void OnCleanDirectory();
	afx_msg void OnDeleteFile();
	afx_msg void OnBnClickedRunProgram();
};
