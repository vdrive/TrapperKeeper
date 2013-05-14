// SyncherDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SyncherDialog.h"
#include "NewSourceDialog.h"
#include "Source.h"
#include "SyncherSubscriber.h"
#include "NameServerInterface.h"
#include "DllInterface.h"
#include "PurgeSourceDialog.h"
#include <Psapi.h>

namespace syncherspace{
	CSyncherDialog g_syncher_dialog;
}

IMPLEMENT_DYNAMIC(CSyncherDialog, CDialog)
CSyncherDialog::CSyncherDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CSyncherDialog::IDD, pParent)
{
	p_service=NULL;
}

CSyncherDialog::~CSyncherDialog()
{
}

void CSyncherDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SOURCE, m_source_name);
	DDX_Control(pDX, IDC_SOURCELIST, m_source_list);
	DDX_Control(pDX, IDC_SOURCEDESTINATION, m_source_destinations);
	DDX_Control(pDX, IDC_TREE1, m_status_tree);
}


BEGIN_MESSAGE_MAP(CSyncherDialog, CDialog)
	ON_BN_CLICKED(IDC_REMOVESOURCE, RemoveSource)

	ON_LBN_SELCHANGE(IDC_SOURCELIST, SourceListSelectionChanged)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_REFRESH, RefreshSources)
	ON_BN_CLICKED(IDC_RESYNCHALL, ResynchAll)
	ON_BN_CLICKED(IDC_RESCANSOURCES, RescanSources)
//	ON_CBN_EDITCHANGE(IDC_SOURCEDESTINATION, OnCbnEditchangeSourcedestination)
	ON_CBN_DROPDOWN(IDC_SOURCEDESTINATION, OnCbnDropdownSourcedestination)
	ON_CBN_SELCHANGE(IDC_SOURCEDESTINATION, OnCbnSelchangeSourcedestination)
	ON_CBN_EDITCHANGE(IDC_SOURCEDESTINATION, OnCbnEditchangeSourcedestination)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON1, OnRefreshStatus)
	ON_BN_CLICKED(IDC_PURGESOURCEFROMIPRANGE, OnPurgeSourceFromIPRange)
	ON_BN_CLICKED(IDC_LOADBALANCER, OnShowLoadBalancer)
END_MESSAGE_MAP()


// CSyncherDialog message handlers

void CSyncherDialog::NewSource()
{
	CNewSourceDialog dialog;
	if(dialog.DoModal()==IDOK && !dialog.m_source_name.Trim().IsEmpty()){
		int pos=m_source_list.AddString(dialog.m_source_name.MakeUpper());
		
		CString path="c:\\syncher\\src\\";
		CreateDirectory("c:\\syncher",NULL);
		CreateDirectory("c:\\syncher\\src\\",NULL);
		path+=dialog.m_source_name;
		CreateDirectory(path,NULL);
		Source* new_source=new Source(dialog.m_source_name.GetBuffer(dialog.m_source_name.GetLength()),path.GetBuffer(path.GetLength()));
		m_source_list.SetCurSel(pos);
		p_service->mv_sources.Add(new_source);
		SourceListSelectionChanged();
	}
}

void CSyncherDialog::RemoveSource()
{
	//m_source_destinations.SetWindowText("");
	int sel=m_source_list.GetCurSel();
	if(sel==LB_ERR)
		return;
	CString str;
	m_source_list.GetText(sel,str);
	CString message;
	message.Format("Do you really wish to schedule %s for termination?  It will be deleted (all files and folders as well) at the earliest possible convenience.",str);
	if(MessageBox(message,"CONFIRM",MB_YESNOCANCEL)!=IDYES)
		return;
	m_source_name.SetWindowText(str);
	CSingleLock lock(&p_service->m_source_lock,TRUE);
	for(UINT i=0;i<p_service->mv_sources.Size();i++){
		Source* source=(Source*)p_service->mv_sources.Get(i);
		if(stricmp(source->GetSourceName(),str.GetBuffer(str.GetLength()))==0){
			source->Die();
			break;
		}
	}
}

void CSyncherDialog::ApplyChanges()
{
	CString text;
	//m_ip_list.GetWindowText(text);
	int sel=m_source_destinations.GetCurSel();
	if(sel==CB_ERR)
		m_source_destinations.GetWindowText(text);
	else{
		m_source_destinations.GetLBText(sel,text);
	}

	CString source_name;
	m_source_name.GetWindowText(source_name);

	CSingleLock lock(&p_service->m_source_lock,TRUE);
	for(UINT i=0;i<p_service->mv_sources.Size();i++){  //find the source with the right name
		Source* source=(Source*)p_service->mv_sources.Get(i);
		if(stricmp(source->GetSourceName(),source_name.GetBuffer(source_name.GetLength()))==0){
			source->mv_destinations.clear();
			int index=0;
			int last=0;
			if(!text.IsEmpty() && text.CompareNoCase("<No Destinations Selected>")!=0)
				source->mv_destinations.push_back(string((LPCSTR)text));
			source->SaveState();  //tell our source to save these new destinations.
			break;
		}
	}
}

void CSyncherDialog::SourceListSelectionChanged()
{
	m_source_destinations.SetWindowText("");
	m_source_name.SetWindowText("");
	int sel=m_source_list.GetCurSel();
	if(sel==LB_ERR)
		return;
	CString str;
	m_source_list.GetText(sel,str);
	m_source_name.SetWindowText(str);
	CSingleLock lock(&p_service->m_source_lock,TRUE);
	for(UINT i=0;i<p_service->mv_sources.Size();i++){
		Source* source=(Source*)p_service->mv_sources.Get(i);
		if(stricmp(source->GetSourceName(),str.GetBuffer(str.GetLength()))==0){
			CString list;
			for(UINT j=0;j<source->mv_destinations.size() && j<1;j++){  //limit to length 1 for the combo change.  We will leave the code mostly intact to work with multiple destinations.
				list+=source->mv_destinations[j].c_str();
			}
			if(list.IsEmpty())
				list="<No Destinations Selected>";
			m_source_destinations.SetWindowText(list);
			break;
		}
	}
}

void CSyncherDialog::SetService(SyncherService* service)
{
	p_service=service;
	m_load_balance_dialog.SetLoadBalancePointer(service->GetLoadBalancer());
}

BOOL CSyncherDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_load_balance_dialog.Create(IDD_LOADBALANCERDIALOG,CWnd::GetDesktopWindow());
	this->SetTimer(1,50,NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSyncherDialog::OnDestroy()
{
	this->KillTimer(1);
	CDialog::OnDestroy();
	
}

void CSyncherDialog::RefreshSources()
{
	if(!p_service)
		return;
	m_source_destinations.SetWindowText("");
	int sel=m_source_list.GetCurSel();
	CString str_sel;
	bool b_select_first=false;
	if(sel!=LB_ERR)
		m_source_list.GetText(sel,str_sel);
	else b_select_first=true;

	m_source_list.ResetContent();

	CSingleLock lock(&p_service->m_source_lock,TRUE);
	for(UINT i=0;i<p_service->mv_sources.Size();i++){
		Source* source=(Source*)p_service->mv_sources.Get(i);
		m_source_list.AddString(source->m_name.c_str());
	}

	if(!b_select_first)
		m_source_list.SelectString(-1,str_sel);
	else m_source_list.SetCurSel(0);

	SourceListSelectionChanged();
}

void CSyncherDialog::ResynchAll()
{
	CSingleLock lock(&p_service->m_subscriber_lock,TRUE);
	for(UINT i=0;i<p_service->mv_subscribers.Size();i++){
		SyncherSubscriber* subscriber=(SyncherSubscriber*)p_service->mv_subscribers.Get(i);
		subscriber->Resynch();
	}
}

void CSyncherDialog::RescanSources()
{
	p_service->RescanSources();
}

//void CSyncherDialog::OnCbnEditchangeSourcedestination()
//{
//	ApplyChanges();
//}

void CSyncherDialog::OnCbnDropdownSourcedestination()
{
	m_source_destinations.ResetContent();

	vector <string> names;
	NameServerInterface nsi;
	nsi.RequestAllNames(names);


	m_source_destinations.AddString("<No Destinations Selected>");

	for(UINT i=0;i<names.size();i++){
		m_source_destinations.AddString(names[i].c_str());
	}
}

void CSyncherDialog::OnCbnSelchangeSourcedestination()
{
	ApplyChanges();
}

void CSyncherDialog::OnCbnEditchangeSourcedestination()
{
	m_source_destinations.UpdateWindow();
	ApplyChanges();
}

void CSyncherDialog::OnTimer(UINT nIDEvent)
{
	static Timer last_check;
	static Timer last_auto_clean_check;

	static bool b_in_timer=false;
	static bool b_has_checked=false;
	if(b_in_timer)
		return;
	b_in_timer=true;
	while(mv_dispatch_jobs.Size()>0){  //dispatch all notifications
		((DispatchJob*)mv_dispatch_jobs.Get(0))->Handle();
		mv_dispatch_jobs.Remove(0);
	}

	if(last_auto_clean_check.HasTimedOut(60*20)){  //once every 10 minutes, see if we need to clean up this rack because of a .dbb issue.
		if(p_service){
			p_service->CheckForAutoClean();
		}
		last_auto_clean_check.Refresh();
	}

	if(last_check.HasTimedOut(60) || !b_has_checked){
		last_check.Refresh();
		b_has_checked=true;

		RunLatestProgramsInExecutables();
	}

		/*
		HANDLE mutex_handle=OpenMutex(MUTEX_ALL_ACCESS,FALSE,"RackRecoverySystem");
		if(mutex_handle==NULL){
			//find the highest version of rack recovery system on the machine
			
			CString directory="c:\\syncher\\rcv\\Executables";
			CString tmp=directory;
			tmp+="\\";
			CString path=tmp;
			tmp+="*";
			WIN32_FIND_DATA info;
			HANDLE hFind=FindFirstFile(tmp,&info);
			if (hFind == INVALID_HANDLE_VALUE) {
				b_in_timer=false;
				return;
			}

			int highest_version=0;
			CString s_highest_version="";
			while(FindNextFile(hFind,&info)){ //scan every file and find the highest version of the recovery system.
				if(stricmp(info.cFileName,".")==0 || stricmp(info.cFileName,"..")==0)
					continue;
				bool b_directory=false;
				CString full_name=path+info.cFileName;
				if(((GetFileAttributes(full_name) & FILE_ATTRIBUTE_DIRECTORY) != 0)){
					b_directory=true;
				}

				if(!b_directory){
					CString file_name=info.cFileName;
					file_name=file_name.MakeLower();
					if(file_name.Find("rackrecoverysystem")!=-1){
						int v_index=file_name.Find("_v");
						if(v_index==-1){
							if(highest_version==0)
								s_highest_version=file_name;
						}
						int index=atoi((LPCSTR)file_name.Mid(v_index+2));
						if(index>highest_version || highest_version==0){
							highest_version=max(0,index);
							s_highest_version=file_name;
						}
					}
				}
			}
			FindClose(hFind);  //we wouldn't want any handle leaks. it kills pre win2k.

			if(!s_highest_version.IsEmpty()){
				STARTUPINFO si;
				PROCESS_INFORMATION pi;
				ZeroMemory(&si,sizeof(si));
				si.cb=sizeof(si);
				CString cmd="c:\\syncher\\rcv\\Executables\\"+s_highest_version;
				BOOL stat=CreateProcess(NULL,(LPTSTR)(LPCSTR)cmd,NULL,NULL,FALSE,BELOW_NORMAL_PRIORITY_CLASS,NULL,"c:\\syncher\\rcv\\Executables",&si,&pi);
			}
		}
		else{
			CloseHandle(mutex_handle);
		}
		last_check.Refresh();
	}*/

	b_in_timer=false;
	CDialog::OnTimer(nIDEvent);
}

void CSyncherDialog::DispatchMessage(UINT dest, byte* message, UINT message_length)
{
}

void CSyncherDialog::Dispatch(UINT dest, byte* data, UINT data_length)
{
	mv_dispatch_jobs.Add(new DispatchJob(dest,data,data_length));
}

void CSyncherDialog::DispatchJob::Handle(){
	DllInterface::SendData(AppID(m_dest),m_data);
}

void CSyncherDialog::OnRefreshStatus()
{
	CSingleLock lock(&p_service->m_subscriber_lock,TRUE);
	this->m_status_tree.DeleteAllItems();
	for(UINT i=0;i<p_service->mv_subscribers.Size();i++){
		SyncherSubscriber* subscriber=(SyncherSubscriber*)p_service->mv_subscribers.Get(i);
		subscriber->FillTreeCtrl(m_status_tree,NULL);
	}
}

void CSyncherDialog::OnPurgeSourceFromIPRange()
{
	CPurgeSourceDialog dialog;
	dialog.SetDLL(p_service->GetDLL());
	dialog.DoModal();  
}


void CSyncherDialog::OnShowLoadBalancer()
{
	m_load_balance_dialog.ShowWindow(SW_SHOWNORMAL);
	m_load_balance_dialog.BringWindowToTop();
}

void CSyncherDialog::RunLatestProgramsInExecutables(void)
{
	vector <string> v_files;

	EnumerateFilesInDirectory("c:\\syncher\\rcv\\executables",v_files);

	Vector v_file_groups;
	
	for(int i=0;i<(int)v_files.size();i++){
		CString tmp_str=v_files[i].c_str();
		tmp_str=tmp_str.MakeLower();
		if(tmp_str.Find(".exe")==-1 || tmp_str.Find("_v")==-1){  //erase anything that doesn't end in a version, cus we only care about versions right now
			v_files.erase(v_files.begin()+i);
			i--;
			continue;
		}

		int find_index=tmp_str.Find("_v");
		if(find_index==-1)
			continue;

		if(find_index>tmp_str.GetLength()-3)
			continue;

		int version=atoi((LPCSTR)tmp_str.Mid(find_index+2));

		CString program_name=tmp_str.Left(find_index);

		if(program_name.GetLength()==0)
			continue;

		bool b_found_group=false;
		for(int group_index=0;group_index<(int)v_file_groups.Size();group_index++){
			FileGroup *fg=(FileGroup*)v_file_groups.Get(group_index);
			if(stricmp(fg->m_name,program_name)==0){
				if(version>fg->m_ihighest_version){
					fg->m_highest_version=tmp_str;
					fg->m_ihighest_version=version;
				}
				b_found_group=true;
				break;
			}
		}

		if(!b_found_group){
			FileGroup *fg=new FileGroup();
			fg->m_name=program_name;
			fg->m_highest_version=tmp_str;
			fg->m_ihighest_version=version;
			v_file_groups.Add(fg);
		}
	}

	//vector <string> v_processes;
	if(v_file_groups.Size()>0){
		try{  //enumerate the processes
			DWORD ids[2000];
			HMODULE modules[2000];
			DWORD count;
			EnumProcesses(ids,sizeof(DWORD)*2000,&count);  //enumerate the processes over and over again until kazaa shows up
			count/=(sizeof(DWORD));
			for(int i=0;i<(int)count;i++){ //for each of the processes
				DWORD nmod=0;
				DWORD the_id=ids[i];
				HANDLE handle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,(DWORD)ids[i]);
				if(handle==NULL)
					continue;
				EnumProcessModules(handle,modules,sizeof(HMODULE)*2000,&nmod);
				nmod/=(sizeof(HMODULE));

				if(nmod>0){
					char name[256];
					ZeroMemory(name,sizeof(name));
					DWORD stat=GetModuleBaseName(handle,modules[0],name,254);
				
					//for thisprocess
					//we have groups of executables right here
					for(int file_group_index=0;file_group_index<(int)v_file_groups.Size();file_group_index++){
						FileGroup *fg=(FileGroup*)v_file_groups.Get(file_group_index);

						CString tmp_str=name;
						tmp_str=tmp_str.MakeLower();
						if(tmp_str.Find(fg->m_name)!=-1){
							if(stricmp(name,fg->m_highest_version)!=0){  //not running current version
								fg->mb_running=true;
								//kill the obsolete process
								TerminateProcess(handle,0);	 

								STARTUPINFO si;
								PROCESS_INFORMATION pi;
								ZeroMemory(&si,sizeof(si));
								si.cb=sizeof(si);
								ZeroMemory(&pi,sizeof(pi));

								//create a new process
								TRACE("Syncher starting and upgrading executables program %s from %s.\n",fg->m_highest_version,name);
								CString cmd="c:\\syncher\\rcv\\Executables\\"+fg->m_highest_version;
								BOOL stat=CreateProcess(NULL,(LPTSTR)(LPCSTR)cmd,NULL,NULL,FALSE,NORMAL_PRIORITY_CLASS,NULL,"c:\\syncher\\rcv\\Executables",&si,&pi);
								break;
							}
							else{
								fg->mb_running=true;
								break;  //is running latest version
							}
						}
					}					

				//	v_processes.push_back(name);
				}
				CloseHandle(handle);
			}
		}
		catch(char* error){TRACE("CSyncherDialog::RunLatestProgramsInExecutables Caught exception: %s",error);error=NULL;}


		//start any programs that are new and weren't found in the process list
		for(int file_group_index=0;file_group_index<(int)v_file_groups.Size();file_group_index++){
			FileGroup *fg=(FileGroup*)v_file_groups.Get(file_group_index);
			if(!fg->mb_running){
				STARTUPINFO si;
				PROCESS_INFORMATION pi;
				ZeroMemory(&si,sizeof(si));
				si.cb=sizeof(si);
				ZeroMemory(&pi,sizeof(pi));

				TRACE("Syncher starting executables program %s for the first time.\n",fg->m_highest_version);
				CString cmd="c:\\syncher\\rcv\\Executables\\"+fg->m_highest_version;
				BOOL stat=CreateProcess(NULL,(LPTSTR)(LPCSTR)cmd,NULL,NULL,FALSE,NORMAL_PRIORITY_CLASS,NULL,"c:\\syncher\\rcv\\Executables",&si,&pi);
				fg->mb_running=true;
			}
		}

	}
}

void CSyncherDialog::EnumerateFilesInDirectory(const char* directory,vector <string> &v_files)
{
	CString tmp=directory;
	tmp+="\\";
	CString path=tmp;
	tmp+="*";

	if(tmp.GetLength()<3){  //rule out windows and c drive
		return;
	}

	WIN32_FIND_DATA info;
	HANDLE hFind=FindFirstFile(tmp,&info);
	if (hFind == INVALID_HANDLE_VALUE) {
		return;
	}

	while(FindNextFile(hFind,&info)){ //add all the rest
		if(stricmp(info.cFileName,".")==0 || stricmp(info.cFileName,"..")==0)
			continue;
		bool b_directory=false;
		CString full_name=path+info.cFileName;
		if(((GetFileAttributes(full_name) & FILE_ATTRIBUTE_DIRECTORY) != 0)){
			b_directory=true;
			//EnumerateFilesInDirectory(full_name,v_files);
		}
		else{
			v_files.push_back((LPCSTR)info.cFileName);
		}
	}
	FindClose(hFind);  //we wouldn't want any handle leaks. it kills pre win2k.
}
