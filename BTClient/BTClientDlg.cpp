// BTClientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTClientDlg.h"
#include "BTClientDll.h"
#include ".\btclientdlg.h"



// BTClientDlg dialog

IMPLEMENT_DYNAMIC(BTClientDlg, CDialog)
BTClientDlg::BTClientDlg(CWnd* pParent /*=NULL*/)
	: CDialog(BTClientDlg::IDD, pParent)
{
}

BTClientDlg::~BTClientDlg()
{
}

void BTClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CLIENT_LIST, m_list_control);
	DDX_Control(pDX, IDC_DATA_LIST, m_data_list_control);
	DDX_Control(pDX, IDC_SENT_LIST, m_sent_list_control);
	DDX_Control(pDX, IDC_Module_Tree, m_module_tree_control);
}


BEGIN_MESSAGE_MAP(BTClientDlg, CDialog)
	ON_WM_TIMER()
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_DATA_LIST, OnLvnColumnclickDataList)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_SENT_LIST, OnLvnColumnclickSentList)
END_MESSAGE_MAP()


BOOL BTClientDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_list_control.InsertColumn(0,"Messages",LVCFMT_LEFT,1000);
	m_data_list_control.InsertColumn(0,"Client IP",LVCFMT_LEFT,100);
	m_data_list_control.InsertColumn(1,"Bytes Downloaded",LVCFMT_RIGHT,100);
	m_data_list_control.InsertColumn(4,"Rate",LVCFMT_RIGHT,50);
	m_data_list_control.InsertColumn(2,"Torrent Filename",LVCFMT_LEFT,200);
	m_data_list_control.InsertColumn(3,"SEED",LVCFMT_CENTER,50);
	m_sent_list_control.InsertColumn(0,"Client IP",LVCFMT_LEFT,100);
	m_sent_list_control.InsertColumn(1,"Bytes Sent",LVCFMT_RIGHT,100);
	m_sent_list_control.InsertColumn(3,"Rate",LVCFMT_RIGHT,50);
	m_sent_list_control.InsertColumn(2,"Torrent Filename",LVCFMT_LEFT,200);
	return true;
}


void BTClientDlg::InitParent(BTClientDll * parent)
{
	p_parent = parent;
}

void BTClientDlg::DisplayInfo(char * info)
{
	CFile logfile;
	
	if (logfile.Open("BTlogfile.txt",CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate|CFile::shareDenyWrite)== TRUE)
	{
		if(logfile.GetLength()> 100000000)
		{
			logfile.Close();
			remove("BTlogfile.txt");
			logfile.Open("BTlogfile.txt",CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate|CFile::shareDenyWrite);
		}
		logfile.SeekToEnd();
		logfile.Write(info, (unsigned int)strlen(info));
		logfile.Write("\r\n",2);
		logfile.Close();
	}

	int index = m_list_control.GetItemCount();
	if (index > 1000)
	{
		m_list_control.DeleteAllItems();
		//m_list_control.DeleteItem(0);
	}
	m_list_control.InsertItem(index,info);
}


void BTClientDlg::OnTimer(UINT nIDEvent)
{
	p_parent->TimerFired(nIDEvent);

}

void BTClientDlg::DisplayClientData(vector<ClientData> cdata)
{
	m_data_list_control.DeleteAllItems();
	m_sent_list_control.DeleteAllItems();
	char intchar[50];
	char doublechar[20];

	double timediff;
	double rate;

	int recindex = 0;
	int sentindex = 0;

	LPARAM lparam = 0;

	for (int index = 0;index < (int)cdata.size(); index++)
	{
		if (cdata[index].m_bytes > 0)
		{
			timediff = difftime((time_t)cdata[index].m_last_time.GetTime(),(time_t)cdata[index].m_first_time.GetTime());	
			rate = cdata[index].m_bytes/timediff;
			sprintf(doublechar," %10.2f ",rate);

			itoa(cdata[index].m_bytes,intchar,10);
			m_data_list_control.InsertItem(recindex,cdata[index].m_ip);
			m_data_list_control.SetItemData(recindex,recindex);
			//m_data_list_control.InsertItem(LVIF_TEXT,recindex,cdata[index].m_ip,0,0,0,lparam);
			m_data_list_control.SetItemText(recindex,1,intchar);
			m_data_list_control.SetItemText(recindex,2,cdata[index].m_torrentname);
			if((cdata[index].m_is_seed) == true)
			{
				m_data_list_control.SetItemText(recindex,3,"YES");
			}
			else
			{
				m_data_list_control.SetItemText(recindex,3,"NO");
			}
			m_data_list_control.SetItemText(recindex,4,doublechar);

			lparam++;
			recindex++;
		}
		if (cdata[index].m_bytes_sent > 0)
		{
			timediff = difftime((time_t)cdata[index].m_last_time.GetTime(),(time_t)cdata[index].m_first_time.GetTime());	
			rate = cdata[index].m_bytes_sent/timediff;
			sprintf(doublechar," %10.2f ",rate);

			itoa(cdata[index].m_bytes_sent,intchar,10);
			m_sent_list_control.InsertItem(sentindex,cdata[index].m_ip);
			m_sent_list_control.SetItemData(sentindex,sentindex);
			m_sent_list_control.SetItemText(sentindex,1,intchar);
			m_sent_list_control.SetItemText(sentindex,2,cdata[index].m_torrentname);
			m_sent_list_control.SetItemText(sentindex,3,doublechar);
			sentindex++;
			lparam++;
		}
	}



	

}


int BTClientDlg::CompareNameProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{

	DWORD_PTR *params=(DWORD_PTR *)lParamSort;
	CListCtrl* pListCtrl =(CListCtrl *)params[0];
    int sub_item=(int)params[1];

    HWND hList = pListCtrl->GetSafeHwnd();


	LVFINDINFO lvfi;
	lvfi.flags = LVFI_PARAM;
    lvfi.lParam = lParam1;
    int index1 = ListView_FindItem(hList, -1, &lvfi);
    lvfi.lParam = lParam2;
    int index2 = ListView_FindItem(hList, -1, &lvfi);




   CString strItem1 = pListCtrl->GetItemText(index1, sub_item);
   CString strItem2 = pListCtrl->GetItemText(index2, sub_item);

   switch(sub_item)
   {
	case 0:
		{
		 if(strItem1.IsEmpty())
         {
             return 1;
         }
         else if(strItem2.IsEmpty())
         {
             return -1;
         }
         else
         {
             return strcmp(strItem1, strItem2);
         }
		}
	case 1:
		{
		   int val1=atoi(strItem1);
		   int val2=atoi(strItem2);
		   if(val1>val2)
		     return 1;
		   else if(val1<val2)
		      return -1;
		   else return 0;
		}
		case 2:
		{
		 if(strItem1.IsEmpty())
         {
             return 1;
         }
         else if(strItem2.IsEmpty())
         {
             return -1;
         }
         else
         {
             return strcmp(strItem1, strItem2);
         }
		}
		case 3:
		{
		 if(strItem1.IsEmpty())
         {
             return 1;
         }
         else if(strItem2.IsEmpty())
         {
             return -1;
         }
         else
         {
             return strcmp(strItem1, strItem2);
         }
		}
		case 4:
		{
		 if(strItem1.IsEmpty())
         {
             return 1;
         }
         else if(strItem2.IsEmpty())
         {
             return -1;
         }
         else
         {
             return strcmp(strItem1, strItem2);
         }
		}

	default:
        {
            return 0;
        }

   }

}


// BTClientDlg message handlers

void BTClientDlg::OnLvnColumnclickDataList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	DWORD_PTR params[2];
    params[0]=(DWORD_PTR)&m_data_list_control;   
    params[1]=pNMLV->iSubItem;     
	m_data_list_control.SortItems(CompareNameProc,(LPARAM)params);
	*pResult = 0;
}

void BTClientDlg::OnLvnColumnclickSentList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	DWORD_PTR params[2];
    params[0]=(DWORD_PTR)&m_sent_list_control;   
    params[1]=pNMLV->iSubItem;     
	m_sent_list_control.SortItems(CompareNameProc,(LPARAM)params);
	*pResult = 0;
}

void BTClientDlg::DisplayModuleData(vector<ModuleData> mdata)
{

	m_module_tree_control.DeleteAllItems();

	HTREEITEM treeroot, treechild;
	char buf[256+1];

	treeroot = m_module_tree_control.InsertItem(_T("testroot"),0,0,TVI_ROOT);
	treechild = m_module_tree_control.InsertItem(_T("testchild"),0,0,treeroot);
	treeroot = m_module_tree_control.InsertItem(_T("testroot2"),0,0,TVI_ROOT);
	treechild = m_module_tree_control.InsertItem(_T("testchild2"),0,0,treeroot);

	for (int i = 0; i < (int)mdata.size(); i++)
	{

		strcpy(buf,mdata[i].m_torrent_name);
		treeroot = m_module_tree_control.InsertItem(buf,0,0,TVI_ROOT);
		for (int j = 0; j <(int)mdata[i].v_ips.size(); j ++)
		{
			strcpy(buf,mdata[i].v_ips[j].c_str());
			treechild = m_module_tree_control.InsertItem(buf,0,0,treeroot);
		}

	}


}