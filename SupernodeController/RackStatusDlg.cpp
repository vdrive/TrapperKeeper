// RackStatusDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SupernodeControllerDll.h"
#include "RackStatusDlg.h"


// CRackStatusDlg dialog

IMPLEMENT_DYNAMIC(CRackStatusDlg, CDialog)
CRackStatusDlg::CRackStatusDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRackStatusDlg::IDD, pParent)
{
}

CRackStatusDlg::~CRackStatusDlg()
{
}

void CRackStatusDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RACK_LIST, m_rack_list);
	DDX_Control(pDX, IDC_SUPERNODE_LIST, m_supernode_list);
}


BEGIN_MESSAGE_MAP(CRackStatusDlg, CDialog)
	ON_NOTIFY(NM_CLICK, IDC_RACK_LIST, OnNMClickRackList)
	ON_NOTIFY(LVN_KEYDOWN, IDC_RACK_LIST, OnLvnKeydownRackList)
END_MESSAGE_MAP()


// CRackStatusDlg message handlers
void CRackStatusDlg::InitParent(SupernodeControllerDll* parent)
{
	p_parent = parent;
}

//
//
//
BOOL CRackStatusDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_rack_list.InsertColumn(0,"Rack Name",LVCFMT_LEFT,100);
//	m_rack_list.InsertColumn(SUB_NUM_KAZAA,"Number of Kazaa",LVCFMT_LEFT,100);
	
	m_supernode_list.InsertColumn(0,"Supernode's IP",LVCFMT_LEFT,200);
	m_supernode_list.InsertColumn(1,"Port",LVCFMT_LEFT,50);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//
//
//
void CRackStatusDlg::AddRack(const char* rack)
{
	int index = m_rack_list.InsertItem(m_rack_list.GetItemCount(),rack);
/*
	char num[4];
	sprintf(num, "%d", rack.m_num_kazaa);
	m_rack_list.SetItemText(index, SUB_NUM_KAZAA, num);
*/
	char msg[32];
	sprintf(msg, "Number of Racks: %d", m_rack_list.GetItemCount());
	GetDlgItem(IDC_RACKS_NUM)->SetWindowText(msg);

}

//
//
//
void GetIPStringFromInterger(int ip_int, char* ip)
{
	sprintf(ip, "%u.%u.%u.%u",(ip_int>>0)&0xFF,(ip_int>>8)&0xFF,(ip_int>>16)&0xFF,(ip_int>>24)&0xFF);
}

//
//
//
void CRackStatusDlg::OnNMClickRackList(NMHDR *pNMHDR, LRESULT *pResult)
{
	//check if the user select any item
	UINT selected = m_rack_list.GetSelectedCount();
	UINT kazaa_running = 0;
	if(selected > 0)
	{
		POSITION pos = m_rack_list.GetFirstSelectedItemPosition();
		int index = m_rack_list.GetNextSelectedItem(pos);
		string rack_name = m_rack_list.GetItemText(index, 0);
		vector<IPAndPort> supernodes = p_parent->GetSupernodesFromRackList(rack_name, kazaa_running);

		//fill up the supernodes list
		m_supernode_list.DeleteAllItems();
		for(unsigned int i=0; i<supernodes.size(); i++)
		{
			char ip[16+1];
			char port[16+1];
			itoa(supernodes[i].m_port, port, 10);
			GetIPStringFromInterger(supernodes[i].m_ip, ip);
			int inserted = m_supernode_list.InsertItem(m_supernode_list.GetItemCount(),ip);
			m_supernode_list.SetItemText(inserted, 1, port);
			/*
			CString up_time = (CTime::GetCurrentTime() - supernodes[i].m_up_since).Format("%D days - %H:%M:%S");
			m_supernode_list.SetItemText(inserted, 2, up_time);
			*/
		}
	}
	
	char msg[32];
	sprintf(msg, "Number of Supernodes: %d", m_supernode_list.GetItemCount());
	GetDlgItem(IDC_SUPERNODE_NUM)->SetWindowText(msg);
	sprintf(msg, "Number of Kazaa: %u", kazaa_running);
	GetDlgItem(IDC_KAZAA_NUM)->SetWindowText(msg);
	*pResult = 0;
}

//
//
//
void CRackStatusDlg::OnLvnKeydownRackList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);

	switch(pLVKeyDow->wVKey)
	{
		case VK_UP:
		{
			//check if the user select any item
			UINT selected = m_rack_list.GetSelectedCount();
			UINT kazaa_running = 0;
			if(selected > 0)
			{
				POSITION pos = m_rack_list.GetFirstSelectedItemPosition();
				int index = m_rack_list.GetNextSelectedItem(pos);
				index--;
				if(index < 0)
					index = 0;
				string rack_name = m_rack_list.GetItemText(index, 0);
				vector<IPAndPort> supernodes = p_parent->GetSupernodesFromRackList(rack_name, kazaa_running);

				//fill up the supernodes list
				m_supernode_list.DeleteAllItems();
				for(unsigned int i=0; i<supernodes.size(); i++)
				{
					char ip[16+1];
					char port[16+1];
					itoa(supernodes[i].m_port, port, 10);
					GetIPStringFromInterger(supernodes[i].m_ip, ip);
					int inserted = m_supernode_list.InsertItem(m_supernode_list.GetItemCount(),ip);
					m_supernode_list.SetItemText(inserted, 1, port);
					/*
					CString up_time = (CTime::GetCurrentTime() - supernodes[i].m_up_since).Format("%D days - %H:%M:%S");
					m_supernode_list.SetItemText(inserted, 2, up_time);
					*/
				}
			}
			
			char msg[32];
			sprintf(msg, "Number of Supernodes: %d", m_supernode_list.GetItemCount());
			GetDlgItem(IDC_SUPERNODE_NUM)->SetWindowText(msg);
			sprintf(msg, "Number of Kazaa: %u", kazaa_running);
			GetDlgItem(IDC_KAZAA_NUM)->SetWindowText(msg);
			break;
		}
		case VK_DOWN:
		{
			//check if the user select any item
			UINT selected = m_rack_list.GetSelectedCount();
			UINT kazaa_running = 0;
			if(selected > 0)
			{
				POSITION pos = m_rack_list.GetFirstSelectedItemPosition();
				int index = m_rack_list.GetNextSelectedItem(pos);
				index++;
				if(index >= m_rack_list.GetItemCount())
					index = m_rack_list.GetItemCount()-1;
				string rack_name = m_rack_list.GetItemText(index, 0);
				vector<IPAndPort> supernodes = p_parent->GetSupernodesFromRackList(rack_name, kazaa_running);

				//fill up the supernodes list
				m_supernode_list.DeleteAllItems();
				for(unsigned int i=0; i<supernodes.size(); i++)
				{
					char ip[16+1];
					char port[16+1];
					itoa(supernodes[i].m_port, port, 10);
					GetIPStringFromInterger(supernodes[i].m_ip, ip);
					int inserted = m_supernode_list.InsertItem(m_supernode_list.GetItemCount(),ip);
					m_supernode_list.SetItemText(inserted, 1, port);
					/*
					CString up_time = (CTime::GetCurrentTime() - supernodes[i].m_up_since).Format("%D days - %H:%M:%S");
					m_supernode_list.SetItemText(inserted, 2, up_time);
					*/
				}
			}
			
			char msg[32];
			sprintf(msg, "Number of Supernodes: %d", m_supernode_list.GetItemCount());
			GetDlgItem(IDC_SUPERNODE_NUM)->SetWindowText(msg);
			sprintf(msg, "Number of Kazaa: %u", kazaa_running);
			GetDlgItem(IDC_KAZAA_NUM)->SetWindowText(msg);
			break;
		}
	}
	*pResult = 0;
}
