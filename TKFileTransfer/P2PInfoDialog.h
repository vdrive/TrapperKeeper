#pragma once
#include "afxcmn.h"
#include "resource.h"
#include "FileTransferService.h"
#include "afxwin.h"

// P2PInfoDialog dialog

class P2PInfoDialog : public CDialog
{
	DECLARE_DYNAMIC(P2PInfoDialog)

private:
	FileTransferService *mp_service;

public:
	P2PInfoDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~P2PInfoDialog();

// Dialog Data
	enum { IDD = IDD_P2PINFODIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CListCtrl m_download_list;
	CListCtrl m_upload_list;
	void SetFileTransferService(FileTransferService* p_service);
	afx_msg void OnTimer(UINT nIDEvent);
	CStatic m_p2p_index_server;
	CStatic m_allocated_buffer;
	CStatic m_files_in_buffer;
};
