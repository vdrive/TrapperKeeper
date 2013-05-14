#pragma once
#include "Resource.h"
#include "afxcmn.h"
#include "XMlParser.h"
#include "afxwin.h"
// CNameServerDlg dialog
#define TAG_COLUMN	0
#define IP_COLUMN	0
class CNameServerDlg : public CDialog
{
	DECLARE_DYNAMIC(CNameServerDlg)

public:
	CNameServerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNameServerDlg();
	void OnCancel();

// Dialog Data
	enum { IDD = IDD_NAME_SERVER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	// List Control that displays ips and the tag associated with it
	virtual BOOL OnInitDialog();
	// Tree that will display all the ips associated with a tag
	CTreeCtrl m_tag_tree;
	void GetParser(XMlParser& parser);
	void LoadTree();
	XMlParser* p_parser;

	afx_msg void OnBnClickedReload();
	// button that will reload the xml file
	CButton m_reload;
};
