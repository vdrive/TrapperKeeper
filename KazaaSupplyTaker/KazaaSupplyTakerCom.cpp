#include "StdAfx.h"
#include "kazaasupplytakercom.h"
#include "kazaasupplytakerdll.h"
#include "DllInterface.h"
#include "../DCMaster/DataBaseInfo.h"
#include "../DCMaster/ProjectKeywordsVector.h"
#include <Dbghelp.h>						// MakeSureDirectoryPathExists()

KazaaSupplyTakerCom::KazaaSupplyTakerCom(void)
{
	m_reading_new_projects = false;
}

KazaaSupplyTakerCom::~KazaaSupplyTakerCom(void)
{
}

void KazaaSupplyTakerCom::InitParent(KazaaSupplyTakerDll *dll)
{
	p_dll = dll;
}

//
// Receives three pieces of data:
//				1) Project Keywords
//				2) KazaaController IP
//				3) DB Connection Info
//
void KazaaSupplyTakerCom::DataReceived(char *source_ip, void *data, UINT data_length)
{
	TRACE("KazaaSupplyTaker: Received Com Data\n");

	//received remote com data

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
		case DCHeader::KazaaControllerIP:
			{
				// update the Kazaa Launcher (supernode) IP address
				p_dll->m_dlg.manager.m_supernode_distributer_ip = ptr;

				if (!p_dll->m_dlg.m_running && !p_dll->m_dlg.m_stop_pressed && p_dll->m_dlg.manager.v_project_keywords.size() > 0)
					p_dll->m_dlg.OnGo();

//				if (p_dll->m_dlg.manager.m_got_sd_ip == false)
//					p_dll->m_dlg.manager.RequestSuperNodeList();
	
				break;
			}
		case DCHeader::DatabaseInfo:
			{
				// Database info, send this info to the KazaaSupplyProcessorDll
				AppID toApp;
				toApp.m_app_id = 6;	// KazaaSupplyProcessor dll's id

				DllInterface::SendData(toApp, ptr, NULL);
				break;
			}
		case DCHeader::ProjectChecksums:
			{
				ProjectChecksums current_projects;
				current_projects.ReadFromBuffer(ptr);

				p_dll->m_dlg.Log(0, NULL, " ");
				p_dll->m_dlg.Log(0, NULL, "Project checksums received, checking for necessary updates/removals");

				// Check to see if we need any new projects, or should delete some; and respond if necessary.
				AnalyzeProjects(source_ip, current_projects);

				break;
			}
		case DCHeader::ProjectKeywords:
			{
				ProjectKeywordsVector keywords;
				keywords.ReadFromBuffer(ptr);

				p_dll->m_dlg.Log(0, NULL, "New Projects Received");

				NewProjectKeywordsReceived(keywords.v_projects);

				if (!p_dll->m_dlg.m_running && !p_dll->m_dlg.m_stop_pressed)
					p_dll->m_dlg.OnGo();

				break;
			}
	}
}

void KazaaSupplyTakerCom::AnalyzeProjects(char *source_ip, ProjectChecksums current_projects)
{
	p_dll->m_dlg.Log(0, NULL, "In Analyze Projects()");
	UINT i;

	ProjectChecksums projects_requiring_update;

	vector<string> current_project_names;
	vector<string> removed_projects;
	
	// check for new projects and/or projects requiring updates
	for(UINT j=0; j<current_projects.v_checksums.size(); j++)
	{
		bool project_found = false;
		for(i=0; i < p_dll->m_dlg.manager.v_project_keywords.size(); i++)
		{
			if(strcmp(current_projects.v_checksums[j].m_project_name.c_str(), p_dll->m_dlg.manager.v_project_keywords[i].m_project_name.c_str())==NULL)
			{
				project_found = true;
				if(p_dll->m_dlg.manager.v_project_keywords[i].CalculateChecksum()!=current_projects.v_checksums[j])
				{
					projects_requiring_update.v_checksums.push_back(current_projects.v_checksums[j]);
				}

				current_project_names.push_back(p_dll->m_dlg.manager.v_project_keywords[i].m_project_name);

				break;
			}
		}

		if(project_found==false)
		{
			projects_requiring_update.v_checksums.push_back(current_projects.v_checksums[j]);
		}
	}

	//determine projects that have been removed
	for(i=0; i < p_dll->m_dlg.manager.v_project_keywords.size(); i++)
	{
		bool project_found = false;
		for(UINT j=0; j<current_project_names.size(); j++)
		{
			if(strcmp(p_dll->m_dlg.manager.v_project_keywords[i].m_project_name.c_str(), current_project_names[j].c_str())==NULL)
			{
				project_found=true;
				break;
			}
		}

		if(project_found==false)
		{
			removed_projects.push_back(p_dll->m_dlg.manager.v_project_keywords[i].m_project_name);
		}
	}

	if(removed_projects.size()>0)
	{
		// Remove the projects removed_projects
		RemoveProjects(removed_projects);
	}

	if(projects_requiring_update.v_checksums.size()>0)
	{
		// Request these projects from the DCMaster
		p_dll->m_dlg.Log(0, NULL, "Requesting Project Updates...");
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
	p_dll->m_dlg.Log(0, NULL, "Exiting Analyze Projects()");
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

	//remove projects from memory
	for(UINT i=0; i < removed_projects.size(); i++)
	{
		vector<ProjectKeywords>::iterator project_iter = p_dll->m_dlg.manager.v_project_keywords.begin();
		for(UINT j=0; j < p_dll->m_dlg.manager.v_project_keywords.size(); j++)
		{
			if(strcmp(p_dll->m_dlg.manager.v_project_keywords[j].m_project_name.c_str(), removed_projects[i].c_str())==NULL)
			{	
				p_dll->m_dlg.manager.v_project_keywords.erase(project_iter);
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
					
			if(strcmp(removed_projects[i].c_str(), file_project_name)==NULL)
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
void KazaaSupplyTakerCom::NewProjectKeywordsReceived(vector<ProjectKeywords> &new_project_keywords)
{
	p_dll->m_dlg.Log(0, NULL, "In NewProjectKeywordsReceived()");

	m_reading_new_projects = true;

	vector<ProjectKeywords> updated_project_keywords;

	//replace updated projects
	for(UINT j=0; j < p_dll->m_dlg.manager.v_project_keywords.size(); j++)
	{
		bool project_found=false;
		for(UINT i=0; i<new_project_keywords.size(); i++)
		{
			if(strcmp(p_dll->m_dlg.manager.v_project_keywords[j].m_project_name.c_str(), new_project_keywords[i].m_project_name.c_str())==NULL)
			{	
				project_found = true;
				updated_project_keywords.push_back(new_project_keywords[i]);
				break;
			}
		}

		if(project_found==false)
		{
			updated_project_keywords.push_back(p_dll->m_dlg.manager.v_project_keywords[j]);
		}
	}

	p_dll->m_dlg.manager.v_project_keywords = updated_project_keywords;

	//add new projects
	for(j=0; j<new_project_keywords.size(); j++)
	{
		bool project_found = false;
		for(UINT i=0; i<p_dll->m_dlg.manager.v_project_keywords.size(); i++)
		{
			if(strcmp(new_project_keywords[j].m_project_name.c_str(), p_dll->m_dlg.manager.v_project_keywords[i].m_project_name.c_str())==NULL)
			{
				project_found =true;
				break;
			}
		}

		if(project_found==false)
		{
			p_dll->m_dlg.manager.v_project_keywords.push_back(new_project_keywords[j]);
		}
	}

	WriteKeywordDataToFile();

	p_dll->m_dlg.Log(0, NULL, "New Project Data Received");

	ReadKeywordDataFromFile();

	m_reading_new_projects = false;
}

//
//
//
void KazaaSupplyTakerCom::WriteKeywordDataToFile()
{
	p_dll->m_dlg.Log(0, NULL, "Writing Keyword Data to Files...");

	CFile keyword_data_file;

	BOOL ret;
	ret = MakeSureDirectoryPathExists("Keyword Files\\");
	ret = MakeSureDirectoryPathExists("Master Keyword Files\\");

	for(UINT i=0; i<p_dll->m_dlg.manager.v_project_keywords.size(); i++)
	{
		// Write the keyword files (the working set of projects)
		BOOL open_write = FALSE;

		char filename[256];
		strcpy(filename, "Keyword Files\\");
		strcat(filename, p_dll->m_dlg.manager.v_project_keywords[i].m_project_name.c_str());
		strcat(filename, ".kwd");
		
		open_write = keyword_data_file.Open(filename,CFile::typeBinary|CFile::modeCreate|CFile::modeWrite|CFile::shareDenyNone);

		if(open_write==TRUE)
		{
			int buf_len = p_dll->m_dlg.manager.v_project_keywords[i].GetBufferLength();
			char *buf = new char[buf_len];

			p_dll->m_dlg.manager.v_project_keywords[i].WriteToBuffer(buf);

			keyword_data_file.Write(buf, buf_len);

			delete [] buf;

			keyword_data_file.Close();
		}

		// Write every project we see to this directory (the master set of projects, for processing)
		// because the DCMaster re-distributes projects every-so-often, which means we need to process
		// everything we've searched on, not just the current working set of projects.
		open_write = FALSE;
		strcpy(filename, "Master Keyword Files\\");
		strcat(filename, p_dll->m_dlg.manager.v_project_keywords[i].m_project_name.c_str());
		strcat(filename, ".kwd");

		open_write = keyword_data_file.Open(filename,CFile::typeBinary|CFile::modeCreate|CFile::modeWrite|CFile::shareDenyNone);

		if(open_write==TRUE)
		{
			int buf_len = p_dll->m_dlg.manager.v_project_keywords[i].GetBufferLength();
			char *buf = new char[buf_len];

			p_dll->m_dlg.manager.v_project_keywords[i].WriteToBuffer(buf);

			keyword_data_file.Write(buf, buf_len);

			delete [] buf;

			keyword_data_file.Close();
		}

	}
}

//
//
//
void KazaaSupplyTakerCom::ReadKeywordDataFromFile()
{
	p_dll->m_dlg.Log(0,NULL, "Reading keyword data from hard drive...");

	char *folder="Keyword Files\\";
	string path;
	
	p_dll->m_dlg.manager.v_project_keywords.clear();
	p_dll->m_dlg.manager.LoadProjects();	// Get misc. projects...

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

			p_dll->m_dlg.manager.v_project_keywords.push_back(new_keywords);

			keyword_data_file.Close();

			delete [] keyword_data;
		}

		found = ::FindNextFile(search_handle, &file_data);
	}
	
	::FindClose(search_handle);

	char msg[1024];
	sprintf(msg, "Read %u projects.", p_dll->m_dlg.manager.v_project_keywords.size());
	p_dll->m_dlg.Log(0, NULL, msg);
	p_dll->m_dlg.Log(0, NULL, " ");
}