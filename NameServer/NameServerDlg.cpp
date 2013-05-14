// SamplePlugInDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NameServerDlg.h"
#include "NameServerInterface.h"
#include "DllInterface.h"

// CNameServerDlg dialog

IMPLEMENT_DYNAMIC(CNameServerDlg, CDialog)
CNameServerDlg::CNameServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNameServerDlg::IDD, pParent)
{
}

CNameServerDlg::~CNameServerDlg()
{
}
void CNameServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE1, m_tag_tree);
	DDX_Control(pDX, IDC_RELOAD, m_reload);
}


BEGIN_MESSAGE_MAP(CNameServerDlg, CDialog)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_RELOAD, OnBnClickedReload)
END_MESSAGE_MAP()


// CNameServerDlg message handlers
void CNameServerDlg::OnCancel()		// user hit ESC
{
	CDialog::OnCancel();
	//OnExit();
}

//
//
//
void CNameServerDlg::OnClose()
{
	ShowWindow(SW_HIDE);	
}

BOOL CNameServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	return false;
}
//used to get the parser object
void CNameServerDlg::GetParser(XMlParser& parser)
{
	p_parser=&parser;
	this->LoadTree();
}

//loads the tree into the dialog box
void CNameServerDlg::LoadTree()
{
	HTREEITEM tag;
	m_tag_tree.DeleteAllItems();
	m_tag_tree.UpdateWindow();
	vector<CString> tag_vector = p_parser->GetTags();	//gets all the unique tags that will be roots in dialog
	vector<string> parser_vector;		//will store all the ip addresses
	for(UINT j=0;j<tag_vector.size();j++)
	{
		tag = m_tag_tree.InsertItem(tag_vector[j],TVI_ROOT,TVI_SORT);	//puts all the tags as roots
		parser_vector = p_parser->GetVector(tag_vector[j].GetBuffer(tag_vector[j].GetLength()));				//gets all the ip addresses for tag_vector[j] tag
		//inserts all the children
		if(parser_vector.size()<10000)	//limits display search results to 10,000 ip addresses
		{
			for(UINT c=0;c<parser_vector.size();c++)
				m_tag_tree.InsertItem(parser_vector[c].c_str(),tag);
		}
		else
		{
			m_tag_tree.InsertItem("List too large to be displayed",tag);	
		}
		m_tag_tree.SortChildren(tag);	//sorts all ip addresses
	}
}

//will reload the xml file
void CNameServerDlg::OnBnClickedReload()
{
	CWaitCursor cursor;		//creates an hour glass 
	m_reload.EnableWindow(FALSE);	//disables the button from further clicking
	p_parser->ReadXMLFile();	//reads the xml file and does all the parsing and data computing
	m_tag_tree.UpdateWindow();	//updates the window
	this->LoadTree();		//reloads the new data into the window
	m_tag_tree.Invalidate(TRUE);	//refreshes the window
	cursor.Restore();		//restores the cursor
	m_reload.EnableWindow(TRUE);	//enables the button to be clicked again
}
