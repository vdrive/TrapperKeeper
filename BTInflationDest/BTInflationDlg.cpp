// BTInflationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTInflatorDll.h"
#include "BTInflationDlg.h"


// BTInflationDlg dialog

IMPLEMENT_DYNAMIC(BTInflationDlg, CDialog)
BTInflationDlg::BTInflationDlg(CWnd* pParent /*=NULL*/)
	: CDialog(BTInflationDlg::IDD, pParent)
{
}

BTInflationDlg::~BTInflationDlg()
{
}

void BTInflationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_Torrent_Tree, m_inflation_tree_ctl);
}


BEGIN_MESSAGE_MAP(BTInflationDlg, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// BTInflationDlg message handlers


void BTInflationDlg::InitParent(BTInflatorDll * parent)
{

	p_parent = parent;

}

void BTInflationDlg::OnTimer(UINT nIDEvent)
{
	p_parent->TimerFired(nIDEvent);

}

void BTInflationDlg::UpdatePeerData(string info_hash, string announce_url, PeerInfo info)
{

	string treeroot = info_hash;
	treeroot +=" ";
	treeroot += announce_url;


}

void BTInflationDlg::RefreshData()
{

	m_inflation_tree_ctl.DeleteAllItems();
/*
	HTREEITEM treeroot, treechild;

	treeroot = m_inflation_tree_ctl.InsertItem(_T("testroot"),0,0,TVI_ROOT);
	treechild = m_inflation_tree_ctl.InsertItem(_T("testchild"),0,0,treeroot);
	treeroot = m_inflation_tree_ctl.InsertItem(_T("testroot2"),0,0,TVI_ROOT);
	treechild = m_inflation_tree_ctl.InsertItem(_T("testchild2"),0,0,treeroot);
*/
}

void BTInflationDlg::UpdateTorrentData(Torrent tor)
{

	HTREEITEM treeroot, treechild;
	string treerootstr = tor.GetName();
	treerootstr +=" ";
	treerootstr += tor.GetInfoHash();
	treerootstr +=" ";
	treerootstr += tor.GetAnnounceUrl();

	treeroot = m_inflation_tree_ctl.InsertItem(_T(treerootstr.c_str()),0,0,TVI_ROOT);
	//treeroot = m_inflation_tree_ctl.InsertItem(_T(treerootstr.c_str()),TVI_ROOT,TVI_LAST);

	vector<PeerInfo> pinfo;
	tor.GetPeerInfo(&pinfo);
	
	char treechildstr[1024+1];
	

	CTimeSpan diff;
	int lastconnect, lastfailure;
	int change = (int)diff.GetTotalMinutes();

	for (int i = 0; i < (int)pinfo.size(); i ++)
	{
		
		diff = CTime::GetCurrentTime()-pinfo[i].m_last_connect;
		lastconnect = (int)diff.GetTotalMinutes();
		diff = CTime::GetCurrentTime()-pinfo[i].m_last_failure;
		lastfailure = (int)diff.GetTotalMinutes();

		sprintf(treechildstr,"%s last success was %i minutes ago, last failure was %i minutes ago seed %i", pinfo[i].m_peer_id.c_str(),lastconnect,lastfailure, pinfo[i].m_is_seed);
		treechild = m_inflation_tree_ctl.InsertItem(_T(treechildstr),0,0,treeroot);
	}


}