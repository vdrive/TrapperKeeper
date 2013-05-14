// AresDataCollectorDialog.cpp : implementation file
//

#include "stdafx.h"
#include "AresDataCollectorDialog.h"
#include "AresDataCollectorSystem.h"
#include "AresDCProjectInfo.h"
#include "AresDCTrackInfo.h"


// CAresDataCollectorDialog dialog

IMPLEMENT_DYNAMIC(CAresDataCollectorDialog, CDialog)
CAresDataCollectorDialog::CAresDataCollectorDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CAresDataCollectorDialog::IDD, pParent)
{
}

CAresDataCollectorDialog::~CAresDataCollectorDialog()
{
}

void CAresDataCollectorDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOG, m_log);
	DDX_Control(pDX, IDC_KNOWNHOSTS, m_known_hosts);
	DDX_Control(pDX, IDC_PROJECTTREE, m_project_tree);
	DDX_Control(pDX, IDC_LIVECONNECTIONS, m_live_connections);
	DDX_Control(pDX, IDC_CONNECTINGSOCKETS, m_connecting_sockets);
	DDX_Control(pDX, IDC_INTERDICTIONTARGETS, m_interdiction_targets);
	DDX_Control(pDX, IDC_LOADED_SUPERNODES, m_loaded_supernodes);
	DDX_Control(pDX, IDC_MD_SERVERS, m_md_servers);
}


BEGIN_MESSAGE_MAP(CAresDataCollectorDialog, CDialog)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_REFRESHPROJECTS, OnRefreshProjectInfo)
END_MESSAGE_MAP()


// CAresDataCollectorDialog message handlers

BOOL CAresDataCollectorDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	this->SetTimer(1,500,NULL);
	this->SetTimer(2,50,NULL);
	this->SetTimer(3,3000,NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CAresDataCollectorDialog::OnDestroy()
{
	CDialog::OnDestroy();

	this->KillTimer(1);
	this->KillTimer(2);
	this->KillTimer(3);
}

void CAresDataCollectorDialog::OnTimer(UINT nIDEvent)
{
	if(nIDEvent==1){
		AresDataCollectorSystemReference ref;
		ref.System()->Update();

		CString tmp_str;
		tmp_str.Format("%u",ref.System()->GetHostCache()->Size());
		m_known_hosts.SetWindowText(tmp_str);
	}
	else if(nIDEvent==2){
		UpdateGUI();
	}
	else if(nIDEvent==3){
		AresDataCollectorSystemReference ref;

		CString tmp_str;
		tmp_str.Format("%u",ref.System()->GetHostCache()->Size());
		m_known_hosts.SetWindowText(tmp_str);

		tmp_str.Format("%u",ref.System()->GetCompletedConnections());
		m_live_connections.SetWindowText(tmp_str);

		tmp_str.Format("%u",ref.System()->GetConnectingSockets());
		m_connecting_sockets.SetWindowText(tmp_str);

		tmp_str.Format("%u",ref.System()->GetInterdictionTargetManager()->GetInterdictionTargetCount());
		m_interdiction_targets.SetWindowText(tmp_str);

		tmp_str.Format("%u",ref.System()->GetSupernodeServerSystem()->GetLoadedSupernodeCount());
		m_loaded_supernodes.SetWindowText(tmp_str);

		tmp_str.Format("%u",ref.System()->GetSupernodeServerSystem()->GetMDServerCount());
		m_md_servers.SetWindowText(tmp_str);
	}

	CDialog::OnTimer(nIDEvent);
}

void CAresDataCollectorDialog::UpdateGUI(void)
{
	AresDataCollectorSystemReference ref;
	Vector v_tmp;
	ref.System()->PopLog(v_tmp);

	if(v_tmp.Size()>0){
		AresDCLogEntry* log_entry=(AresDCLogEntry*)v_tmp.Get(0);
		if(m_log.GetLineCount()>1000){
			//clear out oldest 3/4ths of the buffer
			CString tmp_str;  
			m_log.GetTextRange(3*m_log.GetTextLength()/4,m_log.GetTextLength(),tmp_str);
			m_log.SetWindowText(tmp_str);
		}
		m_log.SetSel(m_log.GetTextLength(),-1);
		m_log.ReplaceSel(log_entry->m_log.c_str());
	}
}

void CAresDataCollectorDialog::OnRefreshProjectInfo()
{
	RefreshProjectTree();
}

void CAresDataCollectorDialog::RefreshProjectTree(void)
{
	m_project_tree.DeleteAllItems();  //clear the tree

	//various HTREEITEMS for references.
	HTREEITEM parent_node1,parent_node2,parent_node3,parent_node4;

	AresDataCollectorSystemReference ref;
	Vector v_tmp_projects;
	AresDCProjectInfoCache* project_cache=ref.System()->GetProjectCache();

	project_cache->GetProjects(v_tmp_projects);

	//go through each of the services
	for(int i=0;i<(int)v_tmp_projects.Size();i++){
		AresDCProjectInfo *pi=(AresDCProjectInfo*)v_tmp_projects.Get(i);

		CString tmp;

		//set project name
		tmp.Format("%d - %s (%s - %s)",i,pi->m_name.c_str(),pi->m_artist.c_str(),pi->m_owner.c_str());
		parent_node1=m_project_tree.InsertItem(tmp,0,0);

		tmp.Format("Search String = '%s'",pi->m_search_string.c_str());
		parent_node2=m_project_tree.InsertItem(tmp,0,0,parent_node1);

		tmp.Format("min size = '%u'",pi->m_size_threashold);
		parent_node2=m_project_tree.InsertItem(tmp,0,0,parent_node1);

		if(pi->mb_decoying)
			parent_node2=m_project_tree.InsertItem("Ares Decoying",0,0,parent_node1);
		if(pi->mb_swarming)
			parent_node2=m_project_tree.InsertItem("Ares Swarming",0,0,parent_node1);


		tmp.Format("Killwords (%d)",pi->mv_killwords.size());
		parent_node2=m_project_tree.InsertItem(tmp,0,0,parent_node1);
		for(int j=0;j<(int)pi->mv_killwords.size();j++){
			m_project_tree.InsertItem(pi->mv_killwords[j].c_str(),0,0,parent_node2);
		}

		tmp.Format("Tracks (%d)",pi->mv_tracks.Size());
		parent_node2=m_project_tree.InsertItem(tmp,0,0,parent_node1);
		for(int j=0;j<(int)pi->mv_tracks.Size();j++){
			AresDCTrackInfo *ti=(AresDCTrackInfo*)pi->mv_tracks.Get(j);
			if(ti->mb_single)
				tmp.Format("%d %s [SINGLE]",ti->m_track_number,ti->m_name.c_str());
			else
				tmp.Format("%d %s",ti->m_track_number,ti->m_name.c_str());
			parent_node3=m_project_tree.InsertItem(tmp,0,0,parent_node2);

			tmp.Format("Keywords (%d)",ti->mv_keywords.size());
			parent_node4=m_project_tree.InsertItem(tmp,0,0,parent_node3);

			for(int k=0;k<(int)ti->mv_keywords.size();k++){
				m_project_tree.InsertItem(ti->mv_keywords[k].c_str(),0,0,parent_node4);
			}
		}
	}

	m_project_tree.Invalidate(TRUE);
}
