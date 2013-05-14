// KeywordManager.cpp

#include "stdafx.h"
#include "KeywordManager.h"
#include "ConnectionManager.h"
#include "GnutellaSupplyDll.h"
#include "imagehlp.h"	// for MakeSureDirectoryPathExists
#include "DBInterface.h"

//
//
//
KeywordManager::KeywordManager()
{
	ReadKeywordDataFromFile();
	v_project_keywords.clear();
	for(UINT i=0; i<v_all_project_keywords.size();i++)
	{
		if(v_all_project_keywords[i].m_project_active == true)
		{
#ifdef SINGLES_ONLY
			bool single_project=false;
			for(UINT j=0; j<v_all_project_keywords[i].m_supply_keywords.v_keywords.size();j++)
			{
				if(v_all_project_keywords[i].m_supply_keywords.v_keywords[j].m_single)
				{
					single_project=true;
					break;
				}
			}

			if(single_project)
#endif
				v_project_keywords.push_back(v_all_project_keywords[i]);
		}
	}
}

//
//
//
KeywordManager::~KeywordManager()
{
	v_project_keywords.clear();
	v_all_project_keywords.clear();
}

//
//
//
void KeywordManager::CheckProjectChecksums(char* dc_master, ProjectChecksums &current_projects)
{
	UINT i;

	ProjectChecksums projects_requiring_update;

	vector<string> current_project_names;
	vector<string> removed_projects;
	
	for(UINT j=0; j<current_projects.v_checksums.size(); j++)
	{
		bool project_found = false;
		for(i=0; i<v_all_project_keywords.size(); i++)
		{
			if(strcmp(current_projects.v_checksums[j].m_project_name.c_str(), v_all_project_keywords[i].m_project_name.c_str())==NULL)
			{
				project_found = true;
				if(v_all_project_keywords[i].CalculateChecksum()!=current_projects.v_checksums[j])
				{
					projects_requiring_update.v_checksums.push_back(current_projects.v_checksums[j]);
				}

				current_project_names.push_back(v_all_project_keywords[i].m_project_name);

				break;
			}
		}

		if(project_found==false)
		{
			projects_requiring_update.v_checksums.push_back(current_projects.v_checksums[j]);
		}
	}

	//determine projects that have been removed
	for(i=0; i<v_all_project_keywords.size(); i++)
	{
		bool project_found = false;
		for(UINT j=0; j<current_project_names.size(); j++)
		{
			if(strcmp(v_all_project_keywords[i].m_project_name.c_str(), current_project_names[j].c_str())==NULL)
			{
				project_found=true;
				break;
			}
		}

		if(project_found==false)
		{
			removed_projects.push_back(v_all_project_keywords[i].m_project_name);
		}
	}

	if(removed_projects.size()>0)
	{
		RemoveProjects(removed_projects);
	}

	if(projects_requiring_update.v_checksums.size()>0)
	{
		p_parent->RequestProjectUpdates(dc_master, projects_requiring_update);
	}
	else
	{
		if(removed_projects.size()>0)
		{
			BroadcastUpdateAlert();
		}
	}

	// Log any new or removed projects
	char log[1024];

	// Check to see if there were any checksums that were sent to us by the beast master that we did not already have
	p_parent->m_log_window_manager.Log("KeywordManager : ",0x00FF40FF);				// purple
	p_parent->m_log_window_manager.Log("Projects Requiring Update : ");
	sprintf(log,"%u\n",projects_requiring_update.v_checksums.size());
	if(projects_requiring_update.v_checksums.size()>0)
	{
		p_parent->m_log_window_manager.Log(log,0x000000FF,true);	// bold red
	}
	else
	{
		p_parent->m_log_window_manager.Log(log,0,true);	// bold
	}

	// Check to see if we have any checksums that were not included within the list that the beast master sent to us
	for(i=0;i<removed_projects.size();i++)
	{
		p_parent->m_log_window_manager.Log("KeywordManager : ",0x00FF40FF);				// purple
		p_parent->m_log_window_manager.Log("Project Removed : ",0x000000FF);	// red
		p_parent->m_log_window_manager.Log((char *)removed_projects[i].c_str(),0x00FF0000,true);	// bold blue
		p_parent->m_log_window_manager.Log("\n");
	}
	p_parent->m_log_window_manager.Log("KeywordManager : ",0x00FF40FF);		// purple
	p_parent->m_log_window_manager.Log("Projects Removed : ");	// red
	sprintf(log,"%u\n",removed_projects.size());
	if(removed_projects.size()>0)
	{
		p_parent->m_log_window_manager.Log(log,0x000000FF,true);	// bold red
	}
	else
	{
		p_parent->m_log_window_manager.Log(log,0,true);	// bold
	}
}


//
//
//
void KeywordManager::RemoveProjects(vector<string> & removed_projects)
{
	vector<ProjectKeywords> current_projects;

	char *folder="Keyword Files\\";
	string path;

	//remove projects from memory
	for(UINT i=0; i<removed_projects.size(); i++)
	{
		vector<ProjectKeywords>::iterator project_iter = v_project_keywords.begin();
		for(UINT j=0; j<v_project_keywords.size(); j++)
		{
			if(strcmp(v_project_keywords[j].m_project_name.c_str(), removed_projects[i].c_str())==NULL)
			{	
				v_project_keywords.erase(project_iter);
				break;
			}

			project_iter++;
		}
		project_iter = v_all_project_keywords.begin();
		for(UINT j=0; j<v_all_project_keywords.size(); j++)
		{
			if(strcmp(v_all_project_keywords[j].m_project_name.c_str(), removed_projects[i].c_str())==NULL)
			{	
				v_all_project_keywords.erase(project_iter);
				break;
			}

			project_iter++;
		}	}

	//remove project files
	WIN32_FIND_DATA file_data;
	path=folder;
	path+="*.kwd";
	HANDLE search_handle = ::FindFirstFile(path.c_str(), &file_data);
	BOOL found = FALSE;

	int count = 0;

	if (search_handle!=INVALID_HANDLE_VALUE)
	{
		found = TRUE;
	}


	while(found == TRUE)
	{
		for(UINT i=0; i<removed_projects.size(); i++)
		{	
			char file_project_name[256+1];
			strcpy(file_project_name, file_data.cFileName);
			/*
			char * temp = strrchr(file_project_name, '.');
			if(temp!=NULL)
			{
				temp[0] = '\0';
			}
			*/
			CString project = removed_projects[i].c_str();
			project.Replace('\\','_');			// replace the backslash with _
			project.Replace('\'', '_');		// replace the single quote "'" with _
			project.Replace(' ', '_');
			project.Replace('-', '_');
			project.Replace('&', '_');
			project.Replace('!', '_');
			project.Replace('$', '_');
			project.Replace('@', '_');
			project.Replace('%', '_');
			project.Replace('(', '_');
			project.Replace(')', '_');
			project.Replace('+', '_');
			project.Replace('~', '_');
			project.Replace('*', '_');
			project.Replace('.', '_');
			project.Replace(',', '_');
			project.Replace('?', '_');
			project.Replace(':', '_');
			project.Replace(';', '_');
			project.Replace('"', '_');
			project.Replace('/', '_');
			project.Replace('#', '_');
			project += ".kwd";

			if(strcmp(project, file_project_name)==NULL)
			{
				path=folder;
				path+=file_data.cFileName;
				CFile::Remove(path.c_str());
				break;
			}
		}

		found = ::FindNextFile(search_handle, &file_data);


	}
	
	::FindClose(search_handle);

}

//
//
//
void KeywordManager::BroadcastUpdateAlert()
{
	//find all the active projects
	v_project_keywords.clear();
	for(UINT i=0; i<v_all_project_keywords.size();i++)
	{
		if(v_all_project_keywords[i].m_project_active == true)
		{
			if(v_all_project_keywords[i].m_gnutella_supply_enabled || v_all_project_keywords[i].m_gnutella_demand_enabled)
			{
#ifdef SINGLES_ONLY
				bool single_project=false;
				for(UINT j=0; j<v_all_project_keywords[i].m_supply_keywords.v_keywords.size();j++)
				{
					if(v_all_project_keywords[i].m_supply_keywords.v_keywords[j].m_single)
					{
						single_project=true;
						break;
					}
				}

				if(single_project)
#endif
					v_project_keywords.push_back(v_all_project_keywords[i]);
			}
		}
	}

	CreateProjectDemandTable();
	CreateProjectSupplyTable();

	p_connection_manager->KeywordsUpdated();
//	p_dist_manager->KeywordsUpdated();
	p_parent->KeywordsUpdated();
//	p_search_manager->KeywordsUpdated();

	vector<SupplyProject> supply_projects;
	for(UINT i=0; i<v_project_keywords.size();i++)
	{
		if(v_project_keywords[i].m_gnutella_supply_enabled)
		{
			SupplyProject project;
			project.m_name=v_project_keywords[i].m_project_name;
			project.m_uber_dist_enabled = v_project_keywords[i].m_uber_disting_enabled;
			project.m_interval=v_project_keywords[i].m_interval;
			supply_projects.push_back(project);
		}
	}
	p_supply_manager->UpdateProjects(supply_projects);

}

//
//
//
void KeywordManager::NewProjectKeywordsReceived(vector<ProjectKeywords> &new_project_keywords)
{
	vector<ProjectKeywords> updated_project_keywords;

	//replace updated projects
	for(UINT j=0; j<v_all_project_keywords.size(); j++)
	{
		bool project_found=false;
		for(UINT i=0; i<new_project_keywords.size(); i++)
		{
			if(strcmp(v_all_project_keywords[j].m_project_name.c_str(), new_project_keywords[i].m_project_name.c_str())==NULL)
			{	
				project_found = true;
				updated_project_keywords.push_back(new_project_keywords[i]);
				break;
			}
		}

		if(project_found==false)
		{
			updated_project_keywords.push_back(v_all_project_keywords[j]);
		}
	}

	v_all_project_keywords = updated_project_keywords;

	//add new projects
	for(j=0; j<new_project_keywords.size(); j++)
	{
		bool project_found = false;
		for(UINT i=0; i<v_all_project_keywords.size(); i++)
		{
			if(strcmp(new_project_keywords[j].m_project_name.c_str(), v_all_project_keywords[i].m_project_name.c_str())==NULL)
			{
				project_found =true;
				break;
			}
		}

		if(project_found==false)
		{
			v_all_project_keywords.push_back(new_project_keywords[j]);
		}
	}

	sort(v_all_project_keywords.begin(),v_all_project_keywords.end());
	WriteKeywordDataToFile();
	BroadcastUpdateAlert();
}

//
//
//
void KeywordManager::WriteKeywordDataToFile()
{
	CFile keyword_data_file;

	BOOL ret=MakeSureDirectoryPathExists("Keyword Files\\");

	for(UINT i=0; i<v_all_project_keywords.size(); i++)
	{
		BOOL open_write = FALSE;

		char filename[256+1];
		strcpy(filename, "Keyword Files\\");

		CString project = v_all_project_keywords[i].m_project_name.c_str();
		project.Replace('\\','_');			// replace the backslash with _
		project.Replace('\'', '_');		// replace the single quote "'" with _
		project.Replace(' ', '_');
		project.Replace('-', '_');
		project.Replace('&', '_');
		project.Replace('!', '_');
		project.Replace('$', '_');
		project.Replace('@', '_');
		project.Replace('%', '_');
		project.Replace('(', '_');
		project.Replace(')', '_');
		project.Replace('+', '_');
		project.Replace('~', '_');
		project.Replace('*', '_');
		project.Replace('.', '_');
		project.Replace(',', '_');
		project.Replace('?', '_');
		project.Replace(':', '_');
		project.Replace(';', '_');
		project.Replace('"', '_');
		project.Replace('#', '_');
		strcat(filename, project);
		strcat(filename, ".kwd");
		
		open_write = keyword_data_file.Open(filename,CFile::typeBinary|CFile::modeCreate|CFile::modeWrite|CFile::shareDenyNone);

		if(open_write==TRUE)
		{
			int buf_len = v_all_project_keywords[i].GetBufferLength();
			char *buf = new char[buf_len];

			v_all_project_keywords[i].WriteToBuffer(buf);

			keyword_data_file.Write(buf, buf_len);

			delete [] buf;

			keyword_data_file.Close();
		}
	}
}

//
//
//
void KeywordManager::ReadKeywordDataFromFile()
{
//	MessageBox(NULL,"KeywordManager::ReadKeywordDataFromFile() - begin","Woo",MB_OK);

	char *folder="Keyword Files\\";
	string path;
	
	v_all_project_keywords.clear();

	WIN32_FIND_DATA file_data;
	path=folder;
	path+="*.kwd";
	HANDLE search_handle = ::FindFirstFile(path.c_str(), &file_data);
	BOOL found = FALSE;

	if (search_handle!=INVALID_HANDLE_VALUE)
	{
		found = TRUE;
	}

	while(found == TRUE)
	{
		CFile keyword_data_file;
		path=folder;
		path+=file_data.cFileName;
		BOOL open = keyword_data_file.Open(path.c_str(),CFile::typeBinary|CFile::modeRead|CFile::shareDenyNone);

		if(open==TRUE)
		{
			unsigned char * keyword_data =  new unsigned char[(UINT)keyword_data_file.GetLength()];

			keyword_data_file.Read(keyword_data, (UINT)keyword_data_file.GetLength());

			ProjectKeywords new_keywords;
			new_keywords.ReadFromBuffer((char*)keyword_data);

			v_all_project_keywords.push_back(new_keywords);

			keyword_data_file.Close();

			delete [] keyword_data;
		}

		found = ::FindNextFile(search_handle, &file_data);
	}
	
	::FindClose(search_handle);
	sort(v_all_project_keywords.begin(),v_all_project_keywords.end());

//	MessageBox(NULL,"KeywordManager::ReadKeywordDataFromFile() - end","Woo",MB_OK);
}

//
//
//
void KeywordManager::SetConnectionManager(ConnectionManager * cm)
{
	p_connection_manager = cm;
}

//
//
//
void KeywordManager::SetSupplyManager(SupplyManager * sm)
{
	p_supply_manager = sm;
}

//
//
//
void KeywordManager::CreateProjectDemandTable()
{
	DBInterface db;
	if(db.OpenDemandConnection("127.0.0.1","onsystems", "ebertsux37", "gnutella_raw_demand"))
	{
		for(UINT i=0; i<v_project_keywords.size(); i++)
		{
			CString project_demand_table = v_project_keywords[i].m_project_name.c_str();
			db.CreateProjectDemandTable(project_demand_table);
		}
		db.CloseDemandConnection();
	}
}

//
//
//
void KeywordManager::CreateProjectSupplyTable()
{
	DBInterface db;
	if(db.OpenSupplyConnection("127.0.0.1","onsystems", "ebertsux37", "gnutella_raw_supply"))
	{
		for(UINT i=0; i<v_project_keywords.size(); i++)
		{
			CString project_supply_table = v_project_keywords[i].m_project_name.c_str();
			db.CreateProjectSupplyTable(project_supply_table);
		}
		db.CloseSupplyConnection();
	}
}