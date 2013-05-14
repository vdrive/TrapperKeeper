#include "StdAfx.h"
#include "../DCMaster/DataBaseInfo.h"
#include "../DCMaster/ProjectKeywordsVector.h"
#include "../DCMaster/DCHeader.h"
#include "KazaaSupplyTakerCom.h"
#include "DllInterface.h"
#include "DistributedKazaaCollectorDll.h"
#include <Dbghelp.h>						// MakeSureDirectoryPathExists()


KazaaSupplyTakerCom::KazaaSupplyTakerCom(void)
{
	m_reading_new_projects = false;
}

KazaaSupplyTakerCom::~KazaaSupplyTakerCom(void)
{
}

void KazaaSupplyTakerCom::InitParent(DistributedKazaaCollectorDll *dll)
{
	p_collector_dll = dll;
}

void KazaaSupplyTakerCom::ComLog(CString msg)
{
	if (m_com_log.Open("Log_File.txt", CFile::modeWrite | CFile::modeCreate | CFile::modeNoTruncate, NULL) != 0)
	{
		m_com_log.SeekToEnd();
		m_com_log.WriteString(msg);
		m_com_log.WriteString("\n");
		m_com_log.Close();
	}
	else
		MessageBox(NULL, "Could not open Com Log file", "Error", MB_OK);
}

//
// Receives three pieces of data:
//				1) Project Keywords
//				2) KazaaController IP
//				3) DB Connection Info
//
void KazaaSupplyTakerCom::DataReceived(char *source_ip, void *data, UINT data_length)
{
	// What kind of message was this?
	char* ptr = (char*)data;
	DCHeader* header = (DCHeader*)ptr;
	ptr+=sizeof(DCHeader);
	switch(header->op)
	{
		case DCHeader::Init:
			{
				// Init message, send responce
				DCHeader header;
				header.op = DCHeader::Kazaa_DC_Init_Response;
				header.size = 0;

				SendReliableData(source_ip, &header, sizeof(header));
				break;
			}
/*
		case DCHeader::KazaaControllerIP:
			{
				// update the Kazaa Launcher (supernode) IP address
				p_collector_dll->m_dlg.manager.m_supernode_distributer_ip = ptr;

				if (!p_collector_dll->m_dlg.m_running && !p_collector_dll->m_dlg.m_stop_pressed && p_collector_dll->v_project_keywords.size() > 0)
					p_collector_dll->m_dlg.OnGo();

//				if (p_collector_dll->m_dlg.manager.m_got_sd_ip == false)
//					p_collector_dll->m_dlg.manager.RequestSuperNodeList();
	
				break;
			}
*/
		case DCHeader::DatabaseInfo:
			{
				// DC Master Database info
				
				//ptr;	// this is pointing to the DB Info
				
				break;
			}
		case DCHeader::ProjectChecksums:
			{
				ProjectChecksums current_projects;
				current_projects.ReadFromBuffer(ptr);

				// Check to see if we need any new projects, or should delete some; and respond if necessary.
				AnalyzeProjects(source_ip, current_projects);

				break;
			}
		case DCHeader::ProjectKeywords:
			{
				ProjectKeywordsVector keywords;
				keywords.ReadFromBuffer(ptr);

				p_collector_dll->m_dlg.Log("New Projects Received - Reseting worker guys to start over...");

				NewProjectKeywordsReceived(keywords.v_projects);

				p_collector_dll->m_dlg.m_last_update = CTime::GetCurrentTime();
				p_collector_dll->ResetWorkersProjectCounts();

				break;
			}
	}
}

void KazaaSupplyTakerCom::AnalyzeProjects(char *source_ip, ProjectChecksums current_projects)
{
	m_reading_new_projects = true;

	UINT i;
	ProjectChecksums projects_requiring_update;

	vector<string> current_project_names;
	vector<string> removed_projects;
	
	// check for new projects and/or projects requiring updates
	for(UINT j=0; j<current_projects.v_checksums.size(); j++)
	{
		bool project_found = false;
		for(i=0; i < p_collector_dll->v_project_keywords.size(); i++)
		{
			if(strcmp(current_projects.v_checksums[j].m_project_name.c_str(), p_collector_dll->v_project_keywords[i].m_project_name.c_str())==NULL)
			{
				project_found = true;
				if(p_collector_dll->v_project_keywords[i].CalculateChecksum()!=current_projects.v_checksums[j])
				{
					projects_requiring_update.v_checksums.push_back(current_projects.v_checksums[j]);
				}

				current_project_names.push_back(p_collector_dll->v_project_keywords[i].m_project_name);

				break;
			}
		}

		if(project_found==false)
		{
			projects_requiring_update.v_checksums.push_back(current_projects.v_checksums[j]);
		}
	}

	//determine projects that have been removed
	for(i=0; i < p_collector_dll->v_project_keywords.size(); i++)
	{
		bool project_found = false;
		for(UINT j=0; j<current_project_names.size(); j++)
		{
			if(strcmp(p_collector_dll->v_project_keywords[i].m_project_name.c_str(), current_project_names[j].c_str())==NULL)
			{
				project_found=true;
				break;
			}
		}

		if(project_found==false)
		{
			removed_projects.push_back(p_collector_dll->v_project_keywords[i].m_project_name);
		}
	}

	if(removed_projects.size()>0)
	{
		// Remove the projects removed_projects
		RemoveProjects(removed_projects);

		char msg[1048];
		sprintf(msg, "Removed %d projects.", removed_projects.size());
		p_collector_dll->m_dlg.Log(msg);
	}

	if(projects_requiring_update.v_checksums.size()>0)
	{
		// Request these projects from the DCMaster
//		p_collector_dll->m_dlg.Log("Requesting Project Updates...");
		RequestProjectUpdates(source_ip, projects_requiring_update);
	}
	else
	{
		if(removed_projects.size()>0)
		{
			// ???
//			BroadcastUpdateAlert();
		}
	}
	
	m_reading_new_projects = false;
}

//
//
//
void KazaaSupplyTakerCom::RequestProjectUpdates(char* dc_master, ProjectChecksums &projects_requiring_update)
{
	int buf_len = projects_requiring_update.GetBufferLength();
	unsigned char* buf = new unsigned char[sizeof(DCHeader)+buf_len];

	DCHeader* header = (DCHeader*)buf;
	header->op = DCHeader::UpdateProjectsRequest;
	header->size = buf_len;

	projects_requiring_update.WriteToBuffer((char*)(&buf[sizeof(DCHeader)]));

	SendReliableData(dc_master, buf, sizeof(DCHeader)+buf_len);

	delete [] buf;
}

//
//
//
void KazaaSupplyTakerCom::RemoveProjects(vector<string> & removed_projects)
{
	vector<ProjectKeywords> current_projects;

	char *folder="Keyword Files\\";
	string path;

	char msg[1048];
	sprintf(msg, "Removing %d projects.", removed_projects.size());
	p_collector_dll->m_dlg.Log(msg);

	//remove projects from memory
	for(UINT i=0; i < removed_projects.size(); i++)
	{
		vector<ProjectKeywords>::iterator project_iter = p_collector_dll->v_project_keywords.begin();
		for(UINT j=0; j < p_collector_dll->v_project_keywords.size(); j++)
		{
			if(strcmp(p_collector_dll->v_project_keywords[j].m_project_name.c_str(), removed_projects[i].c_str())==NULL)
			{	
				p_collector_dll->v_project_keywords.erase(project_iter);
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
			char file_project_name[256];
			strcpy(file_project_name, file_data.cFileName);
			char * temp = strrchr(file_project_name, '.');

			if(temp!=NULL)
			{
				temp[0] = '\0';
			}
				
			CString project = removed_projects[i].c_str();
			project.Replace('\\','_');		// replace the backslash with _
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

	ReadKeywordDataFromFile();
}

//
//
//
void KazaaSupplyTakerCom::NewProjectKeywordsReceived(vector<ProjectKeywords> &new_project_keywords)
{
	m_reading_new_projects = true;

	vector<ProjectKeywords> updated_project_keywords;

	//replace updated projects
	for(UINT j=0; j < p_collector_dll->v_project_keywords.size(); j++)
	{
		bool project_found=false;
		for(UINT i=0; i< new_project_keywords.size(); i++)
		{
			if(strcmp(p_collector_dll->v_project_keywords[j].m_project_name.c_str(), new_project_keywords[i].m_project_name.c_str())==NULL)
			{	
				project_found = true;
				updated_project_keywords.push_back(new_project_keywords[i]);
				break;
			}
		}

		if(project_found==false)
		{
			updated_project_keywords.push_back(p_collector_dll->v_project_keywords[j]);
		}
	}

	p_collector_dll->v_project_keywords = updated_project_keywords;

	//add new projects
	for(j=0; j<new_project_keywords.size(); j++)
	{
		bool project_found = false;
		for(UINT i=0; i< p_collector_dll->v_project_keywords.size(); i++)
		{
			if(strcmp(new_project_keywords[j].m_project_name.c_str(), p_collector_dll->v_project_keywords[i].m_project_name.c_str())==NULL)
			{
				project_found =true;
				break;
			}
		}

		if(project_found==false)
		{
			p_collector_dll->v_project_keywords.push_back(new_project_keywords[j]);
		}
	}

	WriteKeywordDataToFile();

	char msg[1048];
	sprintf(msg, "Added / Updated %d projects.", new_project_keywords.size());
	p_collector_dll->m_dlg.Log(msg);

	ReadKeywordDataFromFile();

	m_reading_new_projects = false;
}

//
//
//
void KazaaSupplyTakerCom::WriteKeywordDataToFile()
{
	CFile keyword_data_file;

	BOOL ret;
	ret = MakeSureDirectoryPathExists("Keyword Files\\");
//	ret = MakeSureDirectoryPathExists("Master Keyword Files\\");

	for(UINT i=0; i<p_collector_dll->v_project_keywords.size(); i++)
	{
		// Write the keyword files (the working set of projects)
		BOOL open_write = FALSE;

		char filename[256];
		strcpy(filename, "Keyword Files\\");

		CString project = p_collector_dll->v_project_keywords[i].m_project_name.c_str();
		project.Replace('\\','_');		// replace the backslash with _
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

		strcat(filename, project);
		strcat(filename, ".kwd");
		
		open_write = keyword_data_file.Open(filename,CFile::typeBinary|CFile::modeCreate|CFile::modeWrite|CFile::shareDenyNone);

		if(open_write==TRUE)
		{
			int buf_len = p_collector_dll->v_project_keywords[i].GetBufferLength();
			char *buf = new char[buf_len];

			p_collector_dll->v_project_keywords[i].WriteToBuffer(buf);

			keyword_data_file.Write(buf, buf_len);

			delete [] buf;

			keyword_data_file.Close();
		}

/*
		// Write every project we see to this directory (the master set of projects, for processing)
		// because the DCMaster re-distributes projects every-so-often, which means we need to process
		// everything we've searched on, not just the current working set of projects.
		open_write = FALSE;
		strcpy(filename, "Master Keyword Files\\");
		strcat(filename, p_collector_dll->v_project_keywords[i].m_project_name.c_str());
		strcat(filename, ".kwd");

		open_write = keyword_data_file.Open(filename,CFile::typeBinary|CFile::modeCreate|CFile::modeWrite|CFile::shareDenyNone);

		if(open_write==TRUE)
		{
			int buf_len = p_collector_dll->v_project_keywords[i].GetBufferLength();
			char *buf = new char[buf_len];

			p_collector_dll->v_project_keywords[i].WriteToBuffer(buf);

			keyword_data_file.Write(buf, buf_len);

			delete [] buf;

			keyword_data_file.Close();
		}
*/

	}
}

//
//
//
void KazaaSupplyTakerCom::ReadKeywordDataFromFile()
{
	char *folder="Keyword Files\\";
	string path;
	
	p_collector_dll->v_project_keywords.clear();
	p_collector_dll->v_project_searched.clear();

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

			p_collector_dll->v_project_keywords.push_back(new_keywords);
			p_collector_dll->v_project_searched.push_back(false);

			keyword_data_file.Close();

			delete [] keyword_data;
		}

		found = ::FindNextFile(search_handle, &file_data);
	}
	
	::FindClose(search_handle);

	char msg[1024];
	sprintf(msg, "Read %u projects.", p_collector_dll->v_project_keywords.size());
	p_collector_dll->m_dlg.Log(msg);

	//
	// Do the project list box inserting
	//
	p_collector_dll->m_dlg.m_project_list.DeleteAllItems();
	UINT size = (UINT)p_collector_dll->v_project_keywords.size();
	for (UINT i=0; i < size; i++)
	{
		int inserted = p_collector_dll->m_dlg.m_project_list.InsertItem(p_collector_dll->m_dlg.m_project_list.GetItemCount(),p_collector_dll->v_project_keywords[i].m_project_name.c_str());
		p_collector_dll->m_dlg.m_project_list.SetItemText(inserted, 1, "0");
		p_collector_dll->m_dlg.m_project_list.SetItemText(inserted, 2, "0");
	}
}