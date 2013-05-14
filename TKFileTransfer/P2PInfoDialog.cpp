// P2PInfoDialog.cpp : implementation file
//

#include "stdafx.h"
#include "P2PInfoDialog.h"


// P2PInfoDialog dialog

IMPLEMENT_DYNAMIC(P2PInfoDialog, CDialog)
P2PInfoDialog::P2PInfoDialog(CWnd* pParent /*=NULL*/)
	: CDialog(P2PInfoDialog::IDD, pParent)
{
}

P2PInfoDialog::~P2PInfoDialog()
{
}

void P2PInfoDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DOWNLOADLIST, m_download_list);
	DDX_Control(pDX, IDC_UPLOADLIST, m_upload_list);
	DDX_Control(pDX, IDC_P2PINDEXSERVER, m_p2p_index_server);
	DDX_Control(pDX, IDC_ALLOCATEDBUFFER, m_allocated_buffer);
	DDX_Control(pDX, IDC_NUMBEROFFILESINBUFFER, m_files_in_buffer);
}


BEGIN_MESSAGE_MAP(P2PInfoDialog, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// P2PInfoDialog message handlers

BOOL P2PInfoDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	this->SetTimer(1,4000,NULL);
	m_download_list.InsertColumn(0,"File Hash",LVCFMT_LEFT,140);
	m_download_list.InsertColumn(1,"File Size",LVCFMT_LEFT,90);
	m_download_list.InsertColumn(3,"Completed Parts/Total Parts",LVCFMT_LEFT,202);
	m_download_list.InsertColumn(4,"Sources",LVCFMT_LEFT,500);

	m_upload_list.InsertColumn(0,"Destination",LVCFMT_LEFT,130);
	m_upload_list.InsertColumn(1,"File Hash",LVCFMT_LEFT,140);
	m_upload_list.InsertColumn(2,"File Size",LVCFMT_LEFT,90);
	m_upload_list.InsertColumn(3,"File Part",LVCFMT_LEFT,75);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void P2PInfoDialog::SetFileTransferService(FileTransferService* p_service)
{
	mp_service=p_service;
}

void P2PInfoDialog::OnTimer(UINT nIDEvent)
{
	//mQueryIncompleteFiles
	//update the incomplete files
	vector <string> v_get_hashes;
	vector <int> v_get_file_sizes;
	vector <int> v_get_completed_parts;
	vector <int> v_get_num_parts;
	vector <string> v_get_sources;
	mp_service->QueryIncompleteP2PJobs(v_get_hashes,v_get_file_sizes,v_get_num_parts,v_get_completed_parts,v_get_sources);
	
	m_download_list.DeleteAllItems();
	for(int i=0;i<(int)v_get_hashes.size() && i<30;i++){
		m_download_list.InsertItem(i,v_get_hashes[i].c_str());
		CString tmp;
		tmp.Format("%d",v_get_file_sizes[i]);
		m_download_list.SetItemText(i,1,tmp);
		tmp.Format("%d / %d",v_get_completed_parts[i],v_get_num_parts[i]);
		m_download_list.SetItemText(i,2,tmp);
		m_download_list.SetItemText(i,3,v_get_sources[i].c_str());
	}


	//update the sends
	vector <int> v_send_parts;
	vector <int> v_send_file_sizes;
	vector <string> v_send_hashes;
	vector <string> v_send_dests;
	mp_service->QuerySendJobs(v_send_dests,v_send_hashes,v_send_file_sizes,v_send_parts);

	m_upload_list.DeleteAllItems();
	for(int i=0;i<(int)v_send_dests.size() && i<30;i++){
		m_upload_list.InsertItem(i,v_send_dests[i].c_str());
		m_upload_list.SetItemText(i,1,v_send_hashes[i].c_str());
		CString tmp;
		tmp.Format("%d",v_send_file_sizes[i]);
		m_upload_list.SetItemText(i,2,tmp);
		tmp.Format("%d",v_send_parts[i]);
		m_upload_list.SetItemText(i,3,tmp);
	}

	m_p2p_index_server.SetWindowText(mp_service->GetP2PIndexServer().c_str());
	CString tmp2;
	tmp2.Format("%u",mp_service->GetAllocatedBuffer());
	m_allocated_buffer.SetWindowText(tmp2);
	tmp2.Format("%d",mp_service->GetBufferFileCount());
	m_files_in_buffer.SetWindowText(tmp2);
	
	CDialog::OnTimer(nIDEvent);
}
