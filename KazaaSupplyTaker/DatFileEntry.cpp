// DatFileEntry

#include "stdafx.h"
#include "DatFileEntry.h"

//
//
//
DatFileEntry::DatFileEntry()
{
	Clear();
}

//
//
//
void DatFileEntry::Clear()
{
	// Proto
	m_internal_ip=0;
	// External IP
	m_port=0;
	m_user_name.empty();
	m_title.empty();
	m_file_name.empty();
	m_file_size=0;
	m_time_stamp=0;
	m_length_in_seconds=0;
	m_resolution_x=0;
	m_resolution_y=0;
	// Company
	// Version of Software
}