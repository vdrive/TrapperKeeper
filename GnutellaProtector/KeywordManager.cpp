// KeywordManager.cpp

#include "stdafx.h"
#include "KeywordManager.h"
#include "ConnectionManager.h"
#include "GnutellaProtectorDll.h"

#include "imagehlp.h"	// for MakeSureDirectoryPathExists

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
			v_project_keywords.push_back(v_all_project_keywords[i]);
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
		//if(v_all_project_keywords[i].m_project_active == true  && v_all_project_keywords[i].m_query_multiplier > 0)
		if(v_all_project_keywords[i].m_project_active == true)
		{
			if(v_all_project_keywords[i].m_gnutella_spoofing_enabled || v_all_project_keywords[i].m_gnutella_swarming_enabled || v_all_project_keywords[i].m_gnutella_decoys_enabled)
			{
				v_project_keywords.push_back(v_all_project_keywords[i]);
				
				//lower case the artist name
				CString artist = v_project_keywords[v_project_keywords.size()-1].m_artist_name.c_str();
				artist.MakeLower();
				v_project_keywords[v_project_keywords.size()-1].m_artist_name = artist;
#ifdef SINGLES_ONLY
				//remove all not single tracks
				vector<SupplyKeyword>::iterator iter=v_project_keywords[v_project_keywords.size()-1].m_supply_keywords.v_keywords.begin();
				while(iter!=v_project_keywords[v_project_keywords.size()-1].m_supply_keywords.v_keywords.end())
				{
					if(!iter->m_single)
						v_project_keywords[v_project_keywords.size()-1].m_supply_keywords.v_keywords.erase(iter);
					else
						iter++;
				}
#endif
				//convert all track title to lower cases
				for(UINT j=0; j<v_project_keywords[v_project_keywords.size()-1].m_supply_keywords.v_keywords.size();j++)
				{
					CString title = v_project_keywords[v_project_keywords.size()-1].m_supply_keywords.v_keywords[j].m_track_name.c_str();
					title.MakeLower();
					if( (title.Find("\'")!=-1) ||  (title.Find("-")!=-1) || (title.Find(".")!=-1) )
					{
						vector<CString> keywords;
						ExtractApostropheDashDotWords(&keywords,title);
						for(UINT k=0;k<keywords.size();k++)
						{
							CString word = keywords[k];
							word.Remove('\'');
							word.Remove('-');
							word.Remove('.');
							title += " ";
							title += word;
						}
					}
					v_project_keywords[v_project_keywords.size()-1].m_supply_keywords.v_keywords[j].m_track_name = title;
				}
			}
		}
	}

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
			project.m_project_id=v_project_keywords[i].m_id;
			project.m_uber_dist_enabled = v_project_keywords[i].m_uber_disting_enabled;
			project.m_interval=v_project_keywords[i].m_interval;
			//find the biggest track number
			project.m_total_tracks = 0;
			for(UINT j=0; j<v_project_keywords[i].m_supply_keywords.v_keywords.size(); j++)
			{
				if(v_project_keywords[i].m_supply_keywords.v_keywords[j].m_track > project.m_total_tracks)
					project.m_total_tracks = v_project_keywords[i].m_supply_keywords.v_keywords[j].m_track;
			}
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
	//remove the file and will start retreiving supply files
#ifdef SPLITTED
	DeleteFile("last_spoof_entries_received.ini");
#endif
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

void KeywordManager::ExtractApostropheDashDotWords(vector<CString> *keywords,const char *title)
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
		if(!found && (strchr(ptr,'\'')!=NULL || strchr(ptr,'-')!=NULL || strchr(ptr,'.')!=NULL) )	// if not, then add it
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