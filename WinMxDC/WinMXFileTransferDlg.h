#pragma once
#include "VirtualListCtrlFT.h"
#include "DlgResizer.h"



// CWinMXFileTransferDlg dialog
class CWinMxDcDlg;
class CWinMXFileTransferDlg : public CDialog
{
	DECLARE_DYNAMIC(CWinMXFileTransferDlg)

public:
	CWinMXFileTransferDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWinMXFileTransferDlg();

// Dialog Data
	enum { IDD = IDD_File_Transfer_DIALOG };
	unsigned int m_num_good_connections;
	unsigned int m_num_bogus_connections;

	void ReportStatus(vector<NoiseModuleThreadStatusData> &status);
	string FormatWithCommas(unsigned int num);
	void InitParent(CWinMxDcDlg* parent);
	void OnUnInitialize();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	// Generated message map functions
	//{{AFX_MSG(CNoiseGeneratorDlg)
	afx_msg void OnPaint();
	afx_msg void OnDisableList();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	unsigned int m_previous_logging_hour;
	unsigned int m_previous_logging_minute;
	unsigned int m_previous_logging_total_num_received_connections;
	unsigned int m_previous_logging_num_good_connections;
	unsigned int m_previous_logging_num_bogus_connections;

private:
	VirtualListCtrlFT m_list;
	CWinMxDcDlg* p_parent;
	CDlgResizer m_DlgResizer;

public:
	afx_msg void OnTimer(UINT nIDEvent);
	virtual BOOL OnInitDialog();
};
