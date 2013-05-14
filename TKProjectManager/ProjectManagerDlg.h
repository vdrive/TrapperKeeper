#pragma once
#include "Resource.h"
#include "afxcmn.h"
#include "..\DCMaster\ProjectKeywords.h"


// CProjectManagerDlg dialog
class CProjectManagerDlg : public CDialog
{
	DECLARE_DYNAMIC(CProjectManagerDlg)

public:
	CProjectManagerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CProjectManagerDlg();
	void UpdateProjectKeyowrdTree(vector<ProjectKeywords>* keywords);
	void SetClient(const char* client);

// Dialog Data
	enum { IDD = IDD_Project_Manager_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CTreeCtrl m_project_keyword_tree;
};
