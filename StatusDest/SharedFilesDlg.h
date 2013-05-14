#pragma once
#include "afxcmn.h"
#include "Resource.h"
#include "..\StatusSource\FileInfos.h"


// CSharedFilesDlg dialog

class CSharedFilesDlg : public CDialog
{
	DECLARE_DYNAMIC(CSharedFilesDlg)

public:
	CSharedFilesDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSharedFilesDlg();

// Dialog Data
	enum { IDD = IDD_SHARED_FILES_DIALOG };

	HWND m_parent_hwnd;
	CString m_ip;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CListCtrl m_files_list;

protected:
	virtual void OnCancel();
public:
	virtual BOOL OnInitDialog();
	void ReceivedSharedFilesReply(FileInfos& reply);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLvnColumnclickFilesList(NMHDR *pNMHDR, LRESULT *pResult);
};
