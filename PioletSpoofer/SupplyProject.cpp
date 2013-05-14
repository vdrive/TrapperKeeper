#include "stdafx.h"
#include "SupplyProject.h"

//
//
//
SupplyProject::SupplyProject()
{
	Clear();	
}

//
//
//
SupplyProject::~SupplyProject()
{
	m_name.erase();
}

//
//
//
SupplyProject::SupplyProject(const SupplyProject &entry)
{
	Clear();
	this->m_interval = entry.m_interval;
	this->m_name = entry.m_name;
	this->m_num_spoofs = entry.m_num_spoofs;
	this->m_offset = entry.m_offset;
	this->m_total_tracks = entry.m_total_tracks;
	this->v_poison_entries = entry.v_poison_entries;
	this->v_spoof_entries = entry.v_spoof_entries;
}

//
//
//
SupplyProject& SupplyProject::operator=(const SupplyProject &entry)
{
	Clear();
	this->m_interval = entry.m_interval;
	this->m_name = entry.m_name;
	this->m_num_spoofs = entry.m_num_spoofs;
	this->m_offset = entry.m_offset;
	this->m_total_tracks = entry.m_total_tracks;
	this->v_poison_entries = entry.v_poison_entries;
	this->v_spoof_entries = entry.v_spoof_entries;
	return *this;
}

//
//
//
void SupplyProject::Clear()
{
	m_interval = 0; // take supply for this project every m_interval minutes
	m_offset = 0;
//	m_uber_dist_enabled = false;
//	m_calculate_supply_counter = 0;
//	memset(&m_query_guid, 0, sizeof(GUID));

	m_num_spoofs = 0;
	m_total_tracks = 0;
	
	v_spoof_entries.clear();
//	v_dist_entries.clear();
	v_poison_entries.clear();
	m_name.erase();
}
	
//
//
//
int SupplyProject::GetBufferLength()
{
	int count = 0;
	count += (int)m_name.size()+1;
//	count += sizeof(m_uber_dist_enabled);
	count += sizeof(m_offset);
	count += sizeof(m_interval);
	count += sizeof(m_num_spoofs);
//	count += sizeof(unsigned int);
	count += sizeof(m_total_tracks);

	count += sizeof(unsigned int);
	for(UINT i=0; i<v_spoof_entries.size(); i++)
	{
		count += v_spoof_entries[i].GetBufferLength();
	}
	count += sizeof(unsigned int);
	for(UINT i=0; i<v_poison_entries.size(); i++)
	{
		count += v_poison_entries[i].GetBufferLength();
	}
/*
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
	UINT i;

	// Clear buffer
	memset(buf,0,GetBufferLength());

	char *ptr=buf;

	strcpy(ptr, m_name.c_str());
	ptr+=m_name.size()+1;

//	*((bool*)ptr)=m_uber_dist_enabled;
//	ptr+=sizeof(bool);

	*((unsigned int *)ptr)=m_offset;
	ptr+=sizeof(unsigned int);

	*((unsigned int *)ptr)=m_interval;
	ptr+=sizeof(unsigned int);

	*((unsigned int *)ptr)=m_num_spoofs;
	ptr+=sizeof(unsigned int);

	*((int *)ptr)=m_total_tracks;
	ptr+=sizeof(int);

	*((unsigned int *)ptr)=(UINT)v_spoof_entries.size();
	ptr+=sizeof(unsigned int);

	for(i=0; i<v_spoof_entries.size(); i++)
	{
		v_spoof_entries[i].WriteToBuffer(ptr);
		ptr+=v_spoof_entries[i].GetBufferLength();
	}

	*((unsigned int *)ptr)=(UINT)v_poison_entries.size();
	ptr+=sizeof(unsigned int);

	for(i=0; i<v_poison_entries.size(); i++)
	{
		v_poison_entries[i].WriteToBuffer(ptr);
		ptr+=v_poison_entries[i].GetBufferLength();
	}
/*
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
	int i;
	int num_entries = 0;

	Clear();

	char *ptr=buf;

	m_name=ptr;
	ptr+=m_name.size()+1;

//	m_uber_dist_enabled = *((bool*)ptr);
//	ptr+=sizeof(bool);

	m_offset=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

	m_interval=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

	m_num_spoofs=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

	m_total_tracks=*((int *)ptr);
	ptr+=sizeof(int);

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
		v_poison_entries.push_back(entry);
	}

/*
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
	char filename[256+1];
	strcpy(filename, "C:\\syncher\\src\\Piolet Supply Files\\");
	CString project =m_name.c_str();
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
	strcat(filename, project);
	strcat(filename, ".sup");

/*
	char new_filename[128];
	strcpy(new_filename,"Supply Files\\");
	strcat(new_filename, m_name.c_str());
	strcat(new_filename, ".arc");

	CFile::Rename(filename, new_filename);
*/

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