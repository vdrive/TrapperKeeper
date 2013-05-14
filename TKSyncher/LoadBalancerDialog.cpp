// LoadBalancerDialog.cpp : implementation file
//

#include "stdafx.h"
#include "LoadBalancerDialog.h"


// LoadBalancerDialog dialog

IMPLEMENT_DYNAMIC(LoadBalancerDialog, CDialog)
LoadBalancerDialog::LoadBalancerDialog(CWnd* pParent /*=NULL*/)
	: CDialog(LoadBalancerDialog::IDD, pParent)
{
	mp_load_balancer=NULL;
}

LoadBalancerDialog::~LoadBalancerDialog()
{
}

void LoadBalancerDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROJECTINFO, m_project_info);
	DDX_Control(pDX, IDC_RACKINFO, m_rack_info);
	DDX_Control(pDX, IDC_DESIRED_RACK_LOAD, m_desired_rack_load);
	DDX_Control(pDX, IDC_ACTUAL_RACK_LOAD, m_actual_rack_load);
	DDX_Control(pDX, IDC_UNASSIGNEDPROJECTS, m_unassigned_projects);
}


BEGIN_MESSAGE_MAP(LoadBalancerDialog, CDialog)
	ON_BN_CLICKED(IDC_REFRESHDATA, OnRefreshData)
END_MESSAGE_MAP()


// LoadBalancerDialog message handlers


void LoadBalancerDialog::OnRefreshData()
{
	CString tmp;
	UINT actual_avg_files=mp_load_balancer->GetActualRackLoad();
	UINT desired_avg_files=mp_load_balancer->GetDesiredRackLoad();

	if(actual_avg_files>0){
		tmp.Format("%d files",actual_avg_files);
	}
	else{
		tmp.Format("N/A");
	}
	m_actual_rack_load.SetWindowText(tmp);

	if(desired_avg_files>0){
		tmp.Format("%d files",desired_avg_files);
	}
	else{
		tmp.Format("N/A");
	}
	m_desired_rack_load.SetWindowText(tmp);

	mp_load_balancer->FillProjectInfoTree(&m_project_info);
	mp_load_balancer->FillRackInfoTree(&m_rack_info);

	m_unassigned_projects.ResetContent();
	vector <string> v_unassigned_projects;
	mp_load_balancer->GetUnassignedProjects(v_unassigned_projects);
	for(int i=0;i<(int)v_unassigned_projects.size();i++){
		const char * tmp=v_unassigned_projects[i].c_str();
		m_unassigned_projects.AddString(tmp);
	}
}

BOOL LoadBalancerDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void LoadBalancerDialog::SetLoadBalancePointer(LoadBalanceSystem* p_system)
{
	mp_load_balancer=p_system;
}
