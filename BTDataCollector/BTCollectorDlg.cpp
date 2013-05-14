// BTCollectorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTDataDll.h"
#include "BTCollectorDlg.h"
#include ".\btcollectordlg.h"


// BTCollectorDlg dialog

IMPLEMENT_DYNAMIC(BTCollectorDlg, CDialog)
BTCollectorDlg::BTCollectorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(BTCollectorDlg::IDD, pParent)
{
}

BTCollectorDlg::~BTCollectorDlg()
{
}

BOOL BTCollectorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_list_control.InsertColumn(0,"Client IP",LVCFMT_LEFT,100);
	m_list_control.InsertColumn(1,"Bytes Downloaded",LVCFMT_RIGHT,100);
	m_list_control.InsertColumn(4,"Rate (Kbps)",LVCFMT_RIGHT,50);
	m_list_control.InsertColumn(2,"Torrent Filename",LVCFMT_LEFT,200);
	m_list_control.InsertColumn(3,"SEED",LVCFMT_CENTER,50);
	m_sent_list_control.InsertColumn(0,"Client IP",LVCFMT_LEFT,100);
	m_sent_list_control.InsertColumn(1,"Bytes Sent",LVCFMT_RIGHT,100);
	m_sent_list_control.InsertColumn(4,"Rate (Kbps)",LVCFMT_RIGHT,50);
	m_sent_list_control.InsertColumn(2,"Torrent Filename",LVCFMT_LEFT,200);
	m_sent_list_control.InsertColumn(3,"SEED",LVCFMT_CENTER,50);
	return true;
}



void BTCollectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX,IDC_MAIN_LIST, m_list_control);
	DDX_Control(pDX,IDC_SENT_LIST, m_sent_list_control);
}


void BTCollectorDlg::InitParent(BTDataDll * parent)
{
	p_parent = parent;
}

void BTCollectorDlg::OnTimer(UINT nIDEvent)
{
	p_parent->TimerFired(nIDEvent);
}


BEGIN_MESSAGE_MAP(BTCollectorDlg, CDialog)
	ON_WM_TIMER()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_SENT_LIST, OnLvnItemchangedSentList)
END_MESSAGE_MAP()


// BTCollectorDlg message handlers


void BTCollectorDlg::DisplayData(vector<ClientData> cdata, vector<double> times)
{
	/*
	m_list_control.DeleteAllItems();
	char intchar[50];
	for (int index = 0;index < (int)cdata.size(); index++)
	{
		itoa(cdata[index].m_bytes,intchar,10);
		m_list_control.InsertItem(index,cdata[index].m_ip);
		m_list_control.SetItemText(index,1,intchar);
		m_list_control.SetItemText(index,2,cdata[index].m_torrentname);
	}
	*/

	m_list_control.DeleteAllItems();
	m_sent_list_control.DeleteAllItems();
	char intchar[50];
	char doublechar[20];

	int recindex = 0;
	int sentindex = 0;
	double rate;

	for (int index = 0;index < (int)cdata.size(); index++)
	{
		ClientData clientd = cdata[index];
		if (cdata[index].m_bytes > 0)
		{
			itoa(cdata[index].m_bytes,intchar,10);
			m_list_control.InsertItem(recindex,cdata[index].m_ip);
			m_list_control.SetItemText(recindex,1,intchar);
			m_list_control.SetItemText(recindex,2,cdata[index].m_torrentname);
			if((cdata[index].m_is_seed) == true)
			{
				m_list_control.SetItemText(recindex,3,"YES");
			}
			else
			{
				m_list_control.SetItemText(recindex,3,"NO");
			}
			rate = cdata[index].m_bytes/times[index];
			sprintf(doublechar," %10.2f ",rate);
			m_list_control.SetItemText(recindex,4,doublechar);

			recindex++;
		}
		if (cdata[index].m_bytes_sent > 0)
		{
			itoa(cdata[index].m_bytes_sent,intchar,10);
			m_sent_list_control.InsertItem(sentindex,cdata[index].m_ip);
			m_sent_list_control.SetItemText(sentindex,1,intchar);
			m_sent_list_control.SetItemText(sentindex,2,cdata[index].m_torrentname);

			rate = cdata[index].m_bytes_sent/times[index];
			sprintf(doublechar," %10.2f ",rate);
			m_sent_list_control.SetItemText(sentindex,4,doublechar);
			sentindex++;

		}
	}

}
void BTCollectorDlg::OnLvnItemchangedSentList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}
