#include "StdAfx.h"
#include "overnetspooferdll.h"
#include "..\DCMaster\DCHeader.h"
#include "DllInterface.h"
#include "..\TKSyncher\TKSyncherInterface.h"


OvernetSpooferDll::OvernetSpooferDll(void)
{
	p_connection_manager=NULL;
}

OvernetSpooferDll::~OvernetSpooferDll(void)
{
}

//
//
//
void OvernetSpooferDll::DllInitialize()
{
	
}

//
//
//
void OvernetSpooferDll::DllUnInitialize()
{
	//m_dlg.DestroyWindow();
	if(p_connection_manager!=NULL)
		delete p_connection_manager;
}

//
//
//
void OvernetSpooferDll::DllStart()
{
	m_dlg.Create(IDD_OVERNET_SPOOFER_MAIN_DIALOG,CWnd::GetDesktopWindow());
	m_dlg.InitParent(this);
	m_dlg.Log("Program started");
	p_connection_manager = new ConnectionManager();
	p_connection_manager->InitParent(this);
	//register com service
	m_project_interface.Init(DCHeader::Overnet_Spoofer_Init_Response);
	m_syncher.Register(this, "OvernetSupplies");
	m_syncher.InitParent(this);

#ifdef SKYCAT
	m_dlg.SetTimer(1,1*1000,0);
#else
#ifdef _DEBUG
	m_dlg.SetTimer(1,30*1000,0);
#else
	m_dlg.SetTimer(1,60*1000,0);
#endif
#endif

	m_dlg.SetTimer(3,1000,0); //1 second
}

//
//
//
void OvernetSpooferDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void OvernetSpooferDll::OnTimer(UINT nIDEvent)
{
	switch(nIDEvent)
	{
		case 1: //2 min
		{
#ifdef SKYCAT
			m_dlg.KillTimer(1);
			ProjectKeywords project;
			project.m_artist_name="skycat";
			project.m_project_name="skycat";
			project.m_id=666666;
			project.m_search_type=ProjectKeywords::search_type::audio;
			QueryKeyword keyword;
			keyword.keyword="skycat";
			keyword.weight=100;
			project.m_query_keywords.v_exact_keywords.push_back(keyword);
			SupplyKeyword s_keyword;
			s_keyword.m_single=true;
			s_keyword.m_track=1;
			s_keyword.m_track_name="rocks";
			char* new_keyword = new char[strlen("rocks")+1];
			strcpy(new_keyword,"rocks");
			s_keyword.v_keywords.push_back(new_keyword);
			project.m_supply_keywords.v_keywords.push_back(s_keyword);
			project.m_supply_keywords.m_supply_size_threshold=1000000;
			project.m_supply_keywords.m_search_string="skycat mp3";
			m_projects.v_projects.push_back(project);
			p_connection_manager->SetProjectKeywords(m_projects);
			p_connection_manager->KeywordsUpdated();

#else
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
						ProjectKeywordsVector projects;
						m_projects.Clear();
						m_project_interface.GetProjectsCopy(projects);
						for(UINT i=0;i<projects.v_projects.size();i++)
						{
#ifdef SPECIAL_PROJECTS
							if(projects.v_projects[i].m_overnet_spoofing_enabled && 
								(projects.v_projects[i].m_id==1937))
#else
							if(projects.v_projects[i].m_overnet_spoofing_enabled)
#endif
							{
								//lower case the artist name
								CString artist = projects.v_projects[i].m_artist_name.c_str();
								artist.MakeLower();
								projects.v_projects[i].m_artist_name = artist;
								
								//remove all not single tracks
								vector<SupplyKeyword>::iterator iter=projects.v_projects[i].m_supply_keywords.v_keywords.begin();
								while(iter!=projects.v_projects[i].m_supply_keywords.v_keywords.end())
								{
									if(!iter->m_single)
										projects.v_projects[i].m_supply_keywords.v_keywords.erase(iter);
									else
										iter++;
								}

								//convert all track title to lower cases
								for(UINT j=0; j<projects.v_projects[i].m_supply_keywords.v_keywords.size();j++)
								{
									CString title =projects.v_projects[i].m_supply_keywords.v_keywords[j].m_track_name.c_str();
									title.MakeLower();
									if( (title.Find("\'")!=-1) ||  (title.Find("-")!=-1) )
									{
										vector<CString> keywords;
										ExtractApostropheDashWords(&keywords,title);
										for(UINT k=0;k<keywords.size();k++)
										{
											CString word = keywords[k];
											word.Remove('\'');
											word.Remove('-');
											title += " ";
											title += word;
										}
									}
									projects.v_projects[i].m_supply_keywords.v_keywords[j].m_track_name = title;
								}


								m_projects.v_projects.push_back(projects.v_projects[i]);
							}
						}
						p_connection_manager->SetProjectKeywords(m_projects);
						p_connection_manager->KeywordsUpdated();
						locked.Unlock();
					}
				}
				m_dlg.SetTimer(2,5*60*1000,0);
			}
#endif
			break;
		}
		case 2: //5 min
		{
			//allowing project setting changes from 12am-6am only
			if(CTime::GetCurrentTime().GetHour() < 6)
			{
				if(m_project_interface.HaveProjectsChanged())
				{
					m_dlg.Log("Projects changed");
					CSingleLock locked(&m_lock);
					locked.Lock();
					if(locked.IsLocked())
					{
						ProjectKeywordsVector projects;
						m_projects.Clear();
						m_project_interface.GetProjectsCopy(projects);
						for(UINT i=0;i<projects.v_projects.size();i++)
						{
#ifdef SPECIAL_PROJECTS
							if(projects.v_projects[i].m_overnet_spoofing_enabled && 
								(projects.v_projects[i].m_id==1937))
#else
							if(projects.v_projects[i].m_overnet_spoofing_enabled)
#endif
							{								//lower case the artist name
								CString artist = projects.v_projects[i].m_artist_name.c_str();
								artist.MakeLower();
								projects.v_projects[i].m_artist_name = artist;
								
								//remove all not single tracks
								vector<SupplyKeyword>::iterator iter=projects.v_projects[i].m_supply_keywords.v_keywords.begin();
								while(iter!=projects.v_projects[i].m_supply_keywords.v_keywords.end())
								{
									if(!iter->m_single)
										projects.v_projects[i].m_supply_keywords.v_keywords.erase(iter);
									else
										iter++;
								}

								//convert all track title to lower cases
								for(UINT j=0; j<projects.v_projects[i].m_supply_keywords.v_keywords.size();j++)
								{
									CString title =projects.v_projects[i].m_supply_keywords.v_keywords[j].m_track_name.c_str();
									title.MakeLower();
									if( (title.Find("\'")!=-1) ||  (title.Find("-")!=-1) )
									{
										vector<CString> keywords;
										ExtractApostropheDashWords(&keywords,title);
										for(UINT k=0;k<keywords.size();k++)
										{
											CString word = keywords[k];
											word.Remove('\'');
											word.Remove('-');
											title += " ";
											title += word;
										}
									}
									projects.v_projects[i].m_supply_keywords.v_keywords[j].m_track_name = title;
								}
								m_projects.v_projects.push_back(projects.v_projects[i]);
							}
						}
						p_connection_manager->SetProjectKeywords(m_projects);
						p_connection_manager->KeywordsUpdated();
						locked.Unlock();
					}
				}
			}
			break;
		}
		case 3: //one sec
		{
			p_connection_manager->TimerHasFired();
			break;
		}
	}
}

//
//
//
void OvernetSpooferDll::Log(const char* log)
{
	m_dlg.Log(log);
}

//
//
//
void OvernetSpooferDll::StatusReady(ConnectionModuleStatusData& status)
{
	status.m_num_projects=(UINT)m_projects.v_projects.size();
	m_dlg.StatusReady(status);
}

//
//
//
void OvernetSpooferDll::GetProjects(ProjectKeywordsVector& projects)
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
void OvernetSpooferDll::SupplySynched(const char* source_ip)
{
 	m_dlg.Log("Supplies are synched, reloading supplies from files");
	p_connection_manager->SupplySynched();
}

//
//
//
void OvernetSpooferDll::ExtractApostropheDashWords(vector<CString> *keywords,const char *title)
{
	UINT i;

	// Make the string lowercase
	char *lbuf=new char[strlen(title)+1];
	strcpy(lbuf,title);
	//strlwr(lbuf);

	// Remove all non alpha-numeric characters
	//ClearNonAlphaNumeric(lbuf,(int)strlen(lbuf));

	// Trim leading and trailing whitespace
	CString cstring=lbuf;
	cstring.TrimLeft();
	cstring.TrimRight();
	cstring.MakeLower();
	//string cstring_string=cstring;
	strcpy(lbuf,cstring);

	// Extract the keywords
	char *ptr=lbuf;
	while(strlen(ptr)>0)
	{
		// Skip past any intermediate spaces in between keywords
		while((*ptr==' '))
		{
			ptr++;
		}

		bool done=true;
		if(strchr(ptr,' ')!=NULL)	// see if there are more keywords after this keyword
		{
			done=false;
			*strchr(ptr,' ')='\0';
		}

		// Check to see if this keyword is already in there
		bool found=false;
		for(i=0;i<keywords->size();i++)
		{
			if(strcmp((*keywords)[i],ptr)==0)
			{
				found=true;
				break;
			}
		}
		if(!found && (strchr(ptr,'\'')!=NULL || strchr(ptr,'-')!=NULL) )	// if not, then add it
		{
			keywords->push_back(ptr);
		}

		if(done)
		{
			break;
		}
		else
		{
			ptr+=strlen(ptr)+1;
		}	
	}
	
	delete [] lbuf;
}