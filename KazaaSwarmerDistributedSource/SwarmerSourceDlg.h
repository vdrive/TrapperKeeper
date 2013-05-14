#pragma once
#include "resource.h"


// CSwarmerSourceDlg dialog
class SwarmerSourceDll;
class CSwarmerSourceDlg : public CDialog
{
	DECLARE_DYNAMIC(CSwarmerSourceDlg)

public:
	CSwarmerSourceDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSwarmerSourceDlg();

	void InitParent(SwarmerSourceDll* parent);

	afx_msg void OnTimer(UINT nIDEvent);
	void DisplayInfo(char * info);
// Dialog Data
	enum { IDD = IDD_DISTRIBUTEDSRC_DLG };
	CListCtrl m_list_control;

	afx_msg void OnBnClickedGetData();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

private:
	SwarmerSourceDll * p_parent;
public:
	afx_msg void OnBnClickedMapButton();
};
