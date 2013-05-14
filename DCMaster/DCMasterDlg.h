#pragma once
#include "afxcmn.h"
#include "Resource.h"
#include "DCStatus.h"
#include "..\GnutellaSupply\DlgResizer.h"

// CDCMasterDlg dialog
class DCMasterDll;
class CDCMasterDlg : public CDialog
{
	DECLARE_DYNAMIC(CDCMasterDlg)

public:
	CDCMasterDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDCMasterDlg();
	void InitParent(DCMasterDll* parent);
	void UpdateCollectorStatus(DCStatus& collector, CString network);
	void ClearProjectsNumber(char* network);
	void ReceivedGiftSupplyStarted(const char* ip);
	void ReceivedGiftSupplyEnded(const char* ip);
	void ReceivedGiftSupplyProgress(const char* ip, int progress);
	void SetDividers(int normal, int singles);
	void SetPioletDivider(int normal);
	void SetBearShareDivider(int normal);
	void SetGiftSwarmerDivider(int normal);
	void SetGiftSpooferDivider(int normal);

// Dialog Data
	enum { IDD = IDD_DC_MASTER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);


	DECLARE_MESSAGE_MAP()
private:
	CListCtrl m_collector_list;
	DCMasterDll* p_parent;
	CDlgResizer m_DlgResizer;

	void UpdateReponseStatus();
	void OnRestartComputer();
	void AlterGnutellaLoadBalancingDivider(int value);
	void AlterGnutellaSinglesLoadBalancingDivider(int value);
	void InitRunningSinceWindowText();
	void AlterPioletLoadBalancingDivider(int value);
	void AlterBearShareLoadBalancingDivider(int value);
	void AlterGiftSwarmerLoadBalancingDivider(int value);
	void AlterGiftSpooferLoadBalancingDivider(int value);


public:
	afx_msg void OnBnClickedResetButton();
	afx_msg void OnTimer(UINT nIDEvent);
	virtual BOOL OnInitDialog();
	afx_msg void OnLvnColumnclickCollectorList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBrowseProjectsButton();
	afx_msg void OnNMRclickCollectorList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposLoadBalancingDividerSpin(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSinglesLoadBalancingSpin(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedResetSplittedGnutellaButton();

	afx_msg void OnBnClickedResetGiftSpooferButton();
	afx_msg void OnBnClickedResetGiftSwarmerButton();
	afx_msg void OnBnClickedResetWinmxSpooferButton();
	afx_msg void OnBnClickedResetWinmxSwarmerButton();
	afx_msg void OnDeltaposPioletLoadBalancingDividerSpin(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedResetSplittedPioletButton();
	afx_msg void OnDeltaposBearshareLoadBalancingDividerSpin(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedResetBearshareButton();
	afx_msg void OnDeltaposGiftSwarmerLoadBalancingDividerSpin(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposGiftSpooferLoadBalancingDividerSpin(NMHDR *pNMHDR, LRESULT *pResult);
};
