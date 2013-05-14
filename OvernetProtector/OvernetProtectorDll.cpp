#include "StdAfx.h"
#include "overnetprotectordll.h"
#include "..\DCMaster\DCHeader.h"
#include "DllInterface.h"
#include "DatabaseHelper.h"
#include "..\TKSyncher\TKSyncherInterface.h"
#include "imagehlp.h"	// for MakeSureDirectoryPathExists


OvernetProtectorDll::OvernetProtectorDll(void)
{
	p_connection_manager=NULL;
	m_retreiving_spoof_entries=false;
}

OvernetProtectorDll::~OvernetProtectorDll(void)
{
}

//
//
//
void OvernetProtectorDll::DllInitialize()
{
	
}

//
//
//
void OvernetProtectorDll::DllUnInitialize()
{
	//m_dlg.DestroyWindow();
	if(p_connection_manager!=NULL)
		delete p_connection_manager;
}

//
//
//
void OvernetProtectorDll::DllStart()
{
	m_dlg.Create(IDD_Overnet_Main_DIALOG,CWnd::GetDesktopWindow());
	m_dlg.InitParent(this);
	m_dlg.Log("Program started");
	p_connection_manager = new ConnectionManager();
	p_connection_manager->InitParent(this);
	//register com service
	//m_com.Register(this,DllInterface::m_app_id.m_app_id);
	m_project_interface.Init(DCHeader::Overnet_DC_Init_Response);
	m_supply.CleanDatabase("Overnet",1);

#ifdef _DEBUG
	m_dlg.SetTimer(1,30*1000,0); //2 min
#else
	m_dlg.SetTimer(1,2*60*1000,0); //2 min
#endif
	m_dlg.SetTimer(3,1000,0); //1 second
	m_dlg.SetTimer(4,60*1000,0); //1 min


}

//
//
//
void OvernetProtectorDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void OvernetProtectorDll::OnTimer(UINT nIDEvent)
{
	switch(nIDEvent)
	{
		case 1: //2 min
		{
			if(m_project_interface.IsReady())
			{
				m_dlg.Log("Projects ready");
				m_dlg.KillTimer(1);
				if(m_project_interface.HaveProjectsChanged())
				{
					CSingleLock locked(&m_lock);
					locked.Lock();
					if(locked.IsLocked())
					{
						m_projects.Clear();

#ifdef SPECIAL_PROJECTS	
						ProjectKeywordsVector projects;
						m_project_interface.GetProjectsCopy(projects);
						for(UINT i=0;i<projects.v_projects.size();i++)
						{
							if(projects.v_projects[i].m_id==3041)
								m_projects.v_projects.push_back(projects.v_projects[i]);
						}
#else
						m_project_interface.GetProjectsCopy(m_projects);
#endif
						p_connection_manager->SetProjectKeywords(m_projects);
						p_connection_manager->KeywordsUpdated();
						locked.Unlock();
					}
				}
				m_dlg.SetTimer(2,5*60*1000,0);
			}
			break;
		}
		case 2: //5 min
		{
			if(m_project_interface.HaveProjectsChanged())
			{
				m_dlg.Log("Projects changed");
				CSingleLock locked(&m_lock);
				locked.Lock();
				if(locked.IsLocked())
				{
					m_projects.Clear();
					m_project_interface.GetProjectsCopy(m_projects);
					p_connection_manager->SetProjectKeywords(m_projects);
					p_connection_manager->KeywordsUpdated();
					locked.Unlock();
				}
			}
			break;
		}
		case 3: //one sec
		{
			p_connection_manager->TimerHasFired();
			break;
		}
		case 4: //one min
		{
			RetreiveNewSpoofEntries();
			break;
		}
	}
}

//
//
//
/*
void OvernetProtectorDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
	char* ptr = (char*)data;
	DCHeader* header = (DCHeader*)ptr;
	ptr+=sizeof(DCHeader);
	
	switch(header->op)
	{
	}
}
*/

//
//
//
void OvernetProtectorDll::Log(const char* log)
{
	m_dlg.Log(log);
}

//
//
//
void OvernetProtectorDll::StatusReady(ConnectionModuleStatusData& status)
{
	m_dlg.StatusReady(status);
	if(status.v_supply_data.size())
		m_supply.SubmitSupplyData(&(status.v_supply_data));
}

//
//
//
void OvernetProtectorDll::GetProjects(ProjectKeywordsVector& projects)
{
	CSingleLock locked(&m_lock);
	locked.Lock();
	if(locked.IsLocked())
	{
		projects = m_projects;
		locked.Unlock();
	}
}

//
//
//
void OvernetProtectorDll::RetrievingProject(const char* project, int track, int percent)
{
	CString msg;
	msg.Format("%s - track %d",project,track);
	m_dlg.RetrievingProject(msg, percent);
}

//
//
//
void OvernetProtectorDll::DoneRetrievingProject()
{
	m_dlg.DoneRetrievingProject();
	//notify the syncher to re-scan the source
	TKSyncherInterface tki;
	tki.RescanSource("OvernetSupplies");
	m_retreiving_spoof_entries = false;
}

UINT SupplyRetrievalThread(LPVOID pParam)
{
	OvernetProtectorDll* parent = (OvernetProtectorDll*)pParam;
	ProjectKeywordsVector projects;
	vector<SupplyData> supplies;
	parent->GetProjects(projects);
	DatabaseHelper database_helper;
#ifdef DC2
	database_helper.Init("127.0.0.1","onsystems","ebertsux37","supply_test");
#else
	database_helper.Init("206.161.141.35","onsystems","ebertsux37","supply_test");
#endif
	UINT buf_len=sizeof(UINT);

	for(UINT i=0; i<projects.v_projects.size();i++)
	{
		if(!projects.v_projects[i].m_supply_keywords.v_keywords.size()) //software,movies i.e. no tracks
		{
			CString query;
			query.Format("select *,sum(count) as total_count from raw_supply where project_id = '%d' and network = '%s' group by hash order by total_count desc limit %d",
				projects.v_projects[i].m_id,"Overnet", 200);
			int percent = (int)((float)(i+1) / (float)projects.v_projects.size() * (float)100);

			parent->RetrievingProject(projects.v_projects[i].m_project_name.c_str(),
				0,percent);
			if(database_helper.Query(query))
			{
				int num_rows = database_helper.GetNumRows();
				for(int i = 0; i < num_rows; i++)
				{
					MYSQL_ROW row = database_helper.GetNextRow();
					SupplyData sd;
					sd.m_filename = row[0];
					sscanf(row[1],"%u",&sd.m_filesize);
					sscanf(row[2],"%u",&sd.m_project_id);
					sd.m_track = atoi(row[3]);
					sd.m_bitrate = atoi(row[4]);
					sd.m_media_len = atoi(row[5]);
					sd.m_hash = row[6];
					sd.m_ip = row[7];
					sd.m_port = atoi(row[8]);
					sd.m_count = atoi(row[9]);
					sd.m_file_type = row[10];
					sd.m_network_name = row[12];
					sd.m_artist = row[13];
					sd.m_title = row[14];
					sd.m_album = row[15];
					sd.m_codec = row[16];
					supplies.push_back(sd);
					buf_len+=sd.GetBufferLength();
				}
			}
			else
			{
				CString log;
				log.Format("SupplyRetrievalThread Error: %s",database_helper.GetLastErrorMessage());
				parent->Log(log);
			}
		}
		else
		{
			for(UINT j=0; j<projects.v_projects[i].m_supply_keywords.v_keywords.size();j++)
			{
				CString query;
				query.Format("select *,sum(count) as total_count from raw_supply where project_id = '%d' and track = %d and network = '%s' group by hash order by total_count desc limit %d",
					projects.v_projects[i].m_id, projects.v_projects[i].m_supply_keywords.v_keywords[j].m_track, "Overnet", 200);
				int percent = (int)((float)(i+1) / (float)projects.v_projects.size() * (float)100);

				parent->RetrievingProject(projects.v_projects[i].m_project_name.c_str(),
					projects.v_projects[i].m_supply_keywords.v_keywords[j].m_track,percent);
				if(database_helper.Query(query))
				{
					int num_rows = database_helper.GetNumRows();
					for(int i = 0; i < num_rows; i++)
					{
						MYSQL_ROW row = database_helper.GetNextRow();
						SupplyData sd;
						sd.m_filename = row[0];
						sscanf(row[1],"%u",&sd.m_filesize);
						sscanf(row[2],"%u",&sd.m_project_id);
						sd.m_track = atoi(row[3]);
						sd.m_bitrate = atoi(row[4]);
						sd.m_media_len = atoi(row[5]);
						sd.m_hash = row[6];
						sd.m_ip = row[7];
						sd.m_port = atoi(row[8]);
						sd.m_count = atoi(row[9]);
						sd.m_file_type = row[10];
						sd.m_network_name = row[12];
						sd.m_artist = row[13];
						sd.m_title = row[14];
						sd.m_album = row[15];
						sd.m_codec = row[16];
						supplies.push_back(sd);
						buf_len+=sd.GetBufferLength();
					}
				}
				else
				{
					CString log;
					log.Format("SupplyRetrievalThread Error: %s",database_helper.GetLastErrorMessage());
					parent->Log(log);
					j--;
				}
			}
		}
	}
	if(supplies.size())
	{
		DeleteFile("c:\\syncher\\src\\OvernetSupplies\\overnet_supplies.dat");
		CFile file;
		MakeSureDirectoryPathExists("c:\\syncher\\src\\OvernetSupplies\\");
		if(file.Open("c:\\syncher\\src\\OvernetSupplies\\overnet_supplies.dat",
			CFile::modeCreate|CFile::typeBinary|CFile::modeWrite|CFile::shareDenyWrite)!=0)
		{
			byte* buf = new byte[buf_len];
			char* ptr = (char*)&buf[0];
			*(UINT*)ptr = (UINT)supplies.size();
			ptr+=sizeof(UINT);
			for(UINT i=0;i<supplies.size();i++)
			{
				ptr+=supplies[i].WriteToBuffer(ptr);
			}
			file.Write(buf,buf_len);
			file.Close();
			delete [] buf;
		}
	}
	parent->DoneRetrievingProject();
	return 0;
}

//
//
//
void OvernetProtectorDll::RetreiveNewSpoofEntries()
{
	if(!m_retreiving_spoof_entries && m_projects.v_projects.size())
	{
		bool retreive_now = false;
		CTime current_time = CTime::GetCurrentTime();
		CFileStatus cFileStatus;
		if(!CFile::GetStatus("c:\\syncher\\src\\OvernetSupplies\\overnet_supplies.dat",cFileStatus))
			retreive_now=true;
		else
		{
			CTime current_time=CTime::GetCurrentTime();
			if( !(cFileStatus.m_mtime.GetYear()==current_time.GetYear() && 
				cFileStatus.m_mtime.GetMonth() == current_time.GetMonth() && 
				cFileStatus.m_mtime.GetDay() ==current_time.GetDay()) )
				retreive_now = true;
		}
		if(retreive_now)
		{
			m_retreiving_spoof_entries = true;
			m_dlg.SpoofEntriesRetrievalStarted();
			AfxBeginThread(SupplyRetrievalThread,(LPVOID)this,THREAD_PRIORITY_BELOW_NORMAL);
		}
	}
}

