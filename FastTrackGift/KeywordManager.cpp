// KeywordManager.cpp

#include "stdafx.h"
#include "KeywordManager.h"
#include "ConnectionManager.h"
#include "FastTrackGiftDll.h"
#include "imagehlp.h"	// for MakeSureDirectoryPathExists
#include "DBInterface.h"
#include "SupplyManager.h"

//
//
//
KeywordManager::KeywordManager()
{
	ReadKeywordDataFromFile();
	v_project_keywords.clear();
	for(UINT i=0; i<v_all_project_keywords.size();i++)
	{
		CString owner = v_all_project_keywords[i].m_owner.c_str();
		owner.MakeLower();

		if(v_all_project_keywords[i].m_project_active == true /*&& owner.Find("catalog")!=-1 // include all audio catalog projects
			&& v_all_project_keywords[i].m_search_type == ProjectKeywords::search_type::audio*/)
		{
#ifdef DATA_COLLECTION_ENABLED
			if(v_all_project_keywords[i].m_gift_supply_enabled)
#endif
#ifdef SHARING_ENABLED
			if(v_all_project_keywords[i].m_gift_spoofing_enabled || v_all_project_keywords[i].m_gift_decoys_enabled)
#endif
			{
				v_project_keywords.push_back(v_all_project_keywords[i]);
			}
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
			//remove the file and will start retreiving supply files
			//DeleteFile("last_spoof_entries_received.ini");
		}
	}

	// Log any new or removed projects
	char log[1024];

	// Check to see if there were any checksums that were sent to us by the beast master that we did not already have
	sprintf(log,"KeywordManager : Projects Requiring Update : %u",projects_requiring_update.v_checksums.size());
	p_parent->Log(log);

	// Check to see if we have any checksums that were not included within the list that the dc master sent to us
	for(i=0;i<removed_projects.size();i++)
	{
		sprintf(log,"KeywordManager : Project Removed : %s",(char *)removed_projects[i].c_str());
		p_parent->Log(log);
	}
	sprintf(log,"KeywordManager : Projects Removed : %d",removed_projects.size());
	p_parent->Log(log);
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
		}
	}

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
		CString owner = v_all_project_keywords[i].m_owner.c_str();
		owner.MakeLower();

		if(v_all_project_keywords[i].m_project_active == true /*&& owner.Find("catalog")!=-1 // include all audio catalog projects
			&& v_all_project_keywords[i].m_search_type == ProjectKeywords::search_type::audio*/)
		{
#ifdef HASH_GROWTH
			v_project_keywords.push_back(v_all_project_keywords[i]);
#else
#ifdef DATA_COLLECTION_ENABLED
			if(v_all_project_keywords[i].m_gift_supply_enabled)
#endif
#ifdef SHARING_ENABLED
			if(v_all_project_keywords[i].m_gift_spoofing_enabled || v_all_project_keywords[i].m_gift_decoys_enabled)
#endif
			{
				v_project_keywords.push_back(v_all_project_keywords[i]);
			}
#endif
		}
	}

	/*
	// testing projects
	ProjectKeywords keywords;
	keywords.m_fasttrack_supply_enabled=true;
	keywords.m_project_active=true;
	keywords.m_interval=10;
	keywords.m_search_type=ProjectKeywords::search_type::audio;
	keywords.m_project_name="MP3";
	keywords.m_supply_keywords.m_search_string="mp3";
	v_project_keywords.push_back(keywords);
	keywords.m_project_name="WMA";
	keywords.m_supply_keywords.m_search_string="wma";
	v_project_keywords.push_back(keywords);
	*/
	
	
	//CreateProjectDemandTable();
#ifndef HASH_GROWTH
#ifdef DATA_COLLECTION_ENABLED
	CreateProjectSupplyTable();
#endif
#else
	CreateDistinctProjectSupplyTable();
#endif

	p_connection_manager->KeywordsUpdated();
	p_parent->KeywordsUpdated();

	vector<SupplyProject> supply_projects;
	for(UINT i=0; i<v_project_keywords.size();i++)
	{
		SupplyProject project;
		project.m_name=v_project_keywords[i].m_project_name;
		project.m_interval=v_project_keywords[i].m_interval;
		project.m_project_id=v_project_keywords[i].m_id;
		supply_projects.push_back(project);
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
	//remove the file and will start retreiving supply files
	DeleteFile("last_spoof_entries_received.ini");
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
		project.Replace('/', '_');
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
/*
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
*/

//
//
//
void KeywordManager::CreateProjectSupplyTable()
{
	DBInterface db;
	if(db.OpenSupplyConnection("127.0.0.1","onsystems", "ebertsux37", "fasttrack_raw_supply"))
	{
		for(UINT i=0; i<v_project_keywords.size(); i++)
		{
			CString project_supply_table = v_project_keywords[i].m_project_name.c_str();
			db.CreateProjectSupplyTable(project_supply_table);
		}
		db.CloseSupplyConnection();
	}
}

//
//
//
void KeywordManager::CreateDistinctProjectSupplyTable()
{
	DBInterface db;
	if(db.OpenSupplyConnection("127.0.0.1","onsystems", "ebertsux37", "fasttrack_raw_supply"))
	{
		for(UINT i=0; i<v_project_keywords.size(); i++)
		{
			CString project_supply_table = v_project_keywords[i].m_project_name.c_str();
			db.CreateDistinctProjectSupplyTable(project_supply_table);
		}
		db.CloseSupplyConnection();
	}
}

//
//
//
vector<ProjectKeywords>* KeywordManager::ReturnProjectKeywords()
{
	return &v_project_keywords;
}
