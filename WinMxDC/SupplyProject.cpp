#include "stdafx.h"
#include "SupplyProject.h"

//
//
//
SupplyProject::SupplyProject()
{
	Clear();	
}

SupplyProject::~SupplyProject()
{
	m_name.erase();
}

//
//
//
void SupplyProject::Clear()
{
	m_interval = 0; // take supply for this project every m_interval minutes
	m_offset = 0;
	m_search_id = 0;

//	m_num_spoofs = 0;
	
//	v_spoof_entries.clear();
//	v_dist_entries.clear();
	m_name.erase();
	m_project_id=0;
}
	
//
//
//
int SupplyProject::GetBufferLength()
{
	int count = 0;
	count += (int)m_name.size()+1;
	count += sizeof(m_offset);
	count += sizeof(m_interval);
	count += sizeof(m_search_id);
	count += sizeof(m_project_id);
//	count += sizeof(m_num_spoofs);
/*
	count += sizeof(unsigned int);

	count += sizeof(unsigned int);

	/*
	for(int i=0; i<v_spoof_entries.size(); i++)
	{
		count += v_spoof_entries[i].GetBufferLength();
	}

	count += sizeof(unsigned int);

	for(i=0; i<v_dist_entries.size(); i++)
	{
		count += v_dist_entries[i].GetBufferLength();
	}
*/
	return count;
}
	
//
//
//
int SupplyProject::WriteToBuffer(char *buf)
{
//	int i;

	// Clear buffer
	memset(buf,0,GetBufferLength());

	char *ptr=buf;

	strcpy(ptr, m_name.c_str());
	ptr+=m_name.size()+1;

	*((unsigned int *)ptr)=m_offset;
	ptr+=sizeof(unsigned int);

	*((unsigned int *)ptr)=m_interval;
	ptr+=sizeof(unsigned int);

	*((unsigned short *)ptr)=m_search_id;
	ptr+=sizeof(unsigned short);

	*((unsigned int *)ptr)=m_project_id;
	ptr+=sizeof(unsigned int);

//	*((unsigned int *)ptr)=m_num_spoofs;
//	ptr+=sizeof(unsigned int);
/*
	*((unsigned int *)ptr)=v_spoof_entries.size();
	ptr+=sizeof(unsigned int);

	for(i=0; i<v_spoof_entries.size(); i++)
	{
		v_spoof_entries[i].WriteToBuffer(ptr);
		ptr+=v_spoof_entries[i].GetBufferLength();
	}

	*((unsigned int *)ptr)=v_dist_entries.size();
	ptr+=sizeof(unsigned int);

	for(i=0; i<v_dist_entries.size(); i++)
	{
		v_dist_entries[i].WriteToBuffer(ptr);
		ptr+=v_dist_entries[i].GetBufferLength();
	}
*/
	return GetBufferLength();
}
	
//
//
//
int SupplyProject::ReadFromBuffer(char *buf)
{
//	int i;
	int num_entries = 0;

	Clear();

	char *ptr=buf;

	m_name=ptr;
	ptr+=m_name.size()+1;

	m_offset=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

	m_interval=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

	m_search_id=*((unsigned short *)ptr);
	ptr+=sizeof(unsigned short);

	m_project_id=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

//	m_num_spoofs=*((unsigned int *)ptr);
//	ptr+=sizeof(unsigned int);
/*
	num_entries=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

	for(i=0; i<num_entries; i++)
	{
		SupplyEntry entry;
		ptr+=entry.ReadFromBuffer(ptr);
		v_spoof_entries.push_back(entry);
	}

	num_entries=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

	for(i=0; i<num_entries; i++)
	{
		SupplyEntry entry;
		ptr+=entry.ReadFromBuffer(ptr);
		v_dist_entries.push_back(entry);
	}
*/
	return GetBufferLength();
}

//
//
//

void SupplyProject::DeleteSupplyFile()
{
	CString filename="Supply Files\\";
	CString project = m_name.c_str();
	project.Replace('\\','_');			// replace the backslash with _
	project.Replace('\'', '_');		// replace the single quote "'" with _
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
	filename += project;
	filename += ".sup";

	try
	{
		CFile::Remove(filename);
	}
	catch(CFileException *e)
	{
		e->Delete();
	}
}

//
//
//
/*
bool SupplyProject::GUIDIsNull()
{
	GUID null_guid;
	memset(&null_guid, 0, sizeof(GUID));
	if(memcmp(&null_guid, &m_query_guid, sizeof(GUID))==NULL)
	{
		return true;
	}
	
	return false;
}
*/