// BrowseProjectsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BrowseProjectsDlg.h"


// CBrowseProjectsDlg dialog

IMPLEMENT_DYNAMIC(CBrowseProjectsDlg, CDialog)
CBrowseProjectsDlg::CBrowseProjectsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBrowseProjectsDlg::IDD, pParent)
{
}

CBrowseProjectsDlg::~CBrowseProjectsDlg()
{
}

void CBrowseProjectsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROJECT_TREE, m_project_tree);
}


BEGIN_MESSAGE_MAP(CBrowseProjectsDlg, CDialog)
END_MESSAGE_MAP()


// CBrowseProjectsDlg message handlers

BOOL CBrowseProjectsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	//HTREEITEM item1;
	//char buf[1024];
	
	// Network names
	//m_project_tree.InsertItem("Gift Swarmer",0,0,TVI_ROOT);
	//m_project_tree.InsertItem("Gift Spoofer",0,0,TVI_ROOT);
	/*
	TVINSERTSTRUCT tvInsert;
	tvInsert.hParent = NULL;
	tvInsert.hInsertAfter = NULL;
	tvInsert.item.mask = TVIF_TEXT;
	tvInsert.item.pszText = _T("Gift Swarmer");
	HTREEITEM hNetwork = m_project_tree.InsertItem(&tvInsert);
	tvInsert.item.pszText = _T("Gift Spoofer");
	hNetwork = m_project_tree.InsertItem(&tvInsert);
	*/


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//
//
//
void CBrowseProjectsDlg::BuildProjectTree(const char* network, vector<DCStatus>& dc_status)
{
	UINT i,j;
	HTREEITEM network_item = m_project_tree.GetNextItem(NULL, TVGN_CHILD);
	if(network_item!=NULL)
	{
		bool found=false;
		while(!found && network_item!=NULL)
		{
			if(strcmp(network,m_project_tree.GetItemText(network_item))==0)
			{
				found=true;
				m_project_tree.DeleteItem(network_item);
			}
			network_item = m_project_tree.GetNextSiblingItem(network_item);
		}
	}

	network_item=m_project_tree.InsertItem(network,0,0,TVI_ROOT);

	for(i=0;i<dc_status.size();i++)
	{
		for(j=0;j<dc_status[i].v_projects.size();j++)
		{
			HTREEITEM project_item = m_project_tree.GetChildItem(network_item);
			if(project_item==NULL)
				project_item = m_project_tree.InsertItem(dc_status[i].v_projects[j].m_project_name.c_str(),network_item,TVI_SORT);
			else
			{
				//project_item=m_project_tree.GetItemText(project_item);
				bool found = false;
				while(!found && project_item!=NULL)
				{
					if(strcmp(dc_status[i].v_projects[j].m_project_name.c_str(),m_project_tree.GetItemText(project_item))==0)
						found=true;
					else
						project_item = m_project_tree.GetNextSiblingItem(project_item);
				}
				if(project_item==NULL)
					project_item = m_project_tree.InsertItem(dc_status[i].v_projects[j].m_project_name.c_str(),network_item,TVI_SORT);
			}
			m_project_tree.InsertItem(dc_status[i].m_ip,project_item,TVI_SORT);
		}
	}
}
