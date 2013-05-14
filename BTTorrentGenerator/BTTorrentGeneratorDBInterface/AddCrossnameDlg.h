#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CAddCrossnameDlg dialog

class Hash20Byte;
class CAddCrossnameDlg : public CDialog
{
	DECLARE_DYNAMIC(CAddCrossnameDlg)

public:
	CAddCrossnameDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAddCrossnameDlg();

// Dialog Data
	enum { IDD = IDD_ADD_CROSSNAME_DLG };

protected:
	CString GetCurrentPath();
	void PopulateFileList(CString path);
	Hash20Byte ComputeInfohash(const char *pFilename);

	virtual void OnOK() {}; 
	virtual void OnCancel() {}; 
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnNMClickFileList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkFileList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnKeydownFileList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedGenerateButton();
	afx_msg void OnBnClickedCrossnameRadio();
	afx_msg void OnBnClickedAddhashRadio();

	DECLARE_MESSAGE_MAP()

protected:
	CListCtrl		m_directoryList;
	CImageList		m_iconImagesList;

	CEdit			m_infileEdit;
	CEdit			m_outfileEdit;
};
