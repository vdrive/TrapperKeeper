#pragma once
#include "VirtualListCtrlFT.h"

// CFileTransferDlg dialog
class CFastTrackGiftDlg;
class CFileTransferDlg : public CDialog
{
	DECLARE_DYNAMIC(CFileTransferDlg)

public:
	CFileTransferDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFileTransferDlg();

// Dialog Data
	enum { IDD = IDD_FILE_TRANSFER_DIALOG };

	unsigned int m_num_good_connections;
	unsigned int m_num_bogus_connections;
	unsigned int m_total_num_received_connections;

	void ReportStatus(vector<NoiseModuleThreadStatusData> &status);
	string FormatWithCommas(unsigned int num);
	void InitParent(CFastTrackGiftDlg* parent);
	void AutoPause(bool pause);

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

	void WriteToLogfile();
private:
	VirtualListCtrlFT m_list;
	CFastTrackGiftDlg* p_parent;
public:
	afx_msg void OnTimer(UINT nIDEvent);
	virtual BOOL OnInitDialog();
};
