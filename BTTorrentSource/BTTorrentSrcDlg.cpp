// BTTorrentSrcDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTTorrentSrcDlg.h"
#include "BTTorrentSourceDll.h"
#include ".\bttorrentsrcdlg.h"


// BTTorrentSrcDlg dialog

IMPLEMENT_DYNAMIC(BTTorrentSrcDlg, CDialog)
BTTorrentSrcDlg::BTTorrentSrcDlg(CWnd* pParent /*=NULL*/)
	: CDialog(BTTorrentSrcDlg::IDD, pParent)
{
}

BTTorrentSrcDlg::~BTTorrentSrcDlg()
{
}

void BTTorrentSrcDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ADD_TORRENT, m_add_torrent_button);
	DDX_Control(pDX, IDC_ANNOUNCE_URL, m_announce_url_ctl);
	DDX_Control(pDX, IDC_INFO_HASH, m_info_hash_ctl);
	DDX_Control(pDX, IDC_INFLATION_LIST, m_inflation_ctl);
	DDX_Control(pDX, IDC_TORRENT_FILENAME, m_torrent_filename_ctl);
}


BEGIN_MESSAGE_MAP(BTTorrentSrcDlg, CDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_ADD_TORRENT, OnBnClickedAddTorrent)
	ON_BN_CLICKED(IDC_REMOVE_BTN, OnBnClickedRemoveBtn)
END_MESSAGE_MAP()


// BTTorrentSrcDlg message handlers


void BTTorrentSrcDlg::InitParent(BTTorrentSourceDll * parent)
{

	p_parent = parent;
	CTime t = CTime::GetCurrentTime();
	srand((unsigned)int(t.GetTime()));
	m_torrentid = 1000000 + rand() % 1000000;

}

void BTTorrentSrcDlg::OnTimer(UINT nIDEvent)
{

	p_parent->TimerFired(nIDEvent);

}
BOOL BTTorrentSrcDlg::OnInitDialog()
{
	BOOL sucess = CDialog::OnInitDialog();
	m_inflation_ctl.InsertColumn(0,"Filename",LVCFMT_LEFT,200);
	m_inflation_ctl.InsertColumn(1,"Hash",LVCFMT_LEFT,150);
	m_inflation_ctl.InsertColumn(2,"ID",LVCFMT_LEFT,100);
	return sucess;
}
void BTTorrentSrcDlg::OnBnClickedAddTorrent()
{
	// TODO: Add your control notification handler code here
	vector<TorrentFileData>  torrent_data;
	TorrentFileData tdata;

	CString announce, info_hash, filename;
	m_announce_url_ctl.GetWindowText(announce);
	m_info_hash_ctl.GetWindowText(info_hash);
	m_torrent_filename_ctl.GetWindowText(filename);


	tdata.m_name = filename;
	tdata.m_announce_URL = announce;
	tdata.m_hash = info_hash;
	tdata.m_torrent_id = m_torrentid;
	m_torrentid +=1;

	m_announce_url_ctl.SetWindowText("");
	m_info_hash_ctl.SetWindowText("");
	m_torrent_filename_ctl.SetWindowText("");

	torrent_data.push_back(tdata);
	p_parent->SendLowSeedTorrentData(torrent_data);
	p_parent->AddNewLowSeedTorrent(tdata);


}

void BTTorrentSrcDlg::OnBnClickedRemoveBtn()
{
	// TODO: Add your control notification handler code here
	int index = m_inflation_ctl.GetSelectionMark();
	if (index != -1)
	{
		string strItem = m_inflation_ctl.GetItemText(index, 2);
		int idnum = atoi(strItem.c_str());
		p_parent->RemoveInflatedTorrent(idnum);
	}


}

void BTTorrentSrcDlg::RefreshList(vector<TorrentFileData> torrentdata)
{

	m_inflation_ctl.DeleteAllItems();
	TorrentFileData tdata; 
	int index;
	for (int i = 0; i < (int)torrentdata.size(); i++)
	{
		char intchar[12];
		tdata = torrentdata[i];
		_itoa(tdata.m_torrent_id,intchar,10);
		index = m_inflation_ctl.InsertItem(i,tdata.m_name.c_str());
		m_inflation_ctl.SetItemData(index,i);
	//	m_inflation_ctl.SetItemText(i,0,tdata.m_name.c_str());
		m_inflation_ctl.SetItemText(index,1,tdata.m_hash.c_str());
		m_inflation_ctl.SetItemText(index,2,intchar);
	}

}

void BTTorrentSrcDlg::RefreshList(vector<TorrentFileData> tdata, vector<TorrentFileData> tdata2)
{

	m_inflation_ctl.DeleteAllItems();
	int i = 0, j = 0;
	for (i = 0; i < (int)tdata.size(); i++)
	{
		char intchar[12];
		_itoa(tdata[i].m_torrent_id,intchar,10);
		m_inflation_ctl.InsertItem(i,tdata[i].m_name.c_str());
		m_inflation_ctl.SetItemData(i,i);
		m_inflation_ctl.SetItemText(i,0,tdata[i].m_name.c_str());
		m_inflation_ctl.SetItemText(i,1,tdata[i].m_hash.c_str());
		m_inflation_ctl.SetItemText(i,2,intchar);
	}

	for (j = 0; j < (int)tdata2.size(); j++)
	{
		char intchar[12];
		_itoa(tdata2[j].m_torrent_id,intchar,10);
		m_inflation_ctl.InsertItem(i+j,tdata2[j].m_name.c_str());
		m_inflation_ctl.SetItemData(i+j,i+j);
		m_inflation_ctl.SetItemText(i+j,0,tdata2[j].m_name.c_str());
		m_inflation_ctl.SetItemText(i+j,1,tdata2[j].m_hash.c_str());
		m_inflation_ctl.SetItemText(i+j,2,intchar);
	}

}