#pragma once
#include "resource.h"
#include "afxcmn.h"
#include "DllInfo.h"

#define SUB_DLL_NAME		0
#define SUB_DLL_APPID		1
#define SUB_DLL_VERSION		2

// CDllLoaderDlg dialog
class DllLoaderApp;
class CDllLoaderDlg : public CDialog
{
	DECLARE_DYNAMIC(CDllLoaderDlg)

public:
	CDllLoaderDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDllLoaderDlg();
	void OnCancel();

	BOOL OnCommand(WPARAM wParam,LPARAM lParam);

// Dialog Data
	enum { IDD = IDD_TRAPPER_KEEPER_DIALOG };

protected:
	HICON m_hIcon;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	DllLoaderApp* p_app;
	int m_border;
	int m_bottom;
public:
	void InitParent(DllLoaderApp* parent);
	afx_msg void OnBnClickedShowGuiButton();
//	afx_msg void OnDestroy();
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	CListCtrl m_dll_list;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	void UpdateDllList(vector<DllInfo>& dlls);
	afx_msg void OnNMDblclkDllList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRclickDllList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnColumnclickDllList(NMHDR *pNMHDR, LRESULT *pResult);
};
