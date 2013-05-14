#include "StdAfx.h"
#include "datfilerecord.h"

DatfileRecord::DatfileRecord(void)
{

	memset(m_filename,0,sizeof(m_filename));
	memset(m_hash,0,sizeof(m_hash));
	m_size = 0;
	memset(m_datfilename,0,sizeof(m_datfilename));
	m_time = NULL;

}

DatfileRecord::~DatfileRecord(void)
{
}


void DatfileRecord::ClearRecord()
{
	memset(m_filename,0,sizeof(m_filename));
	memset(m_hash,0,sizeof(m_hash));
	m_size = 0;
	memset(m_datfilename,0,sizeof(m_datfilename));
	m_time = NULL;
}

void DatfileRecord::SetRecord(char * filename, char * hash, int filesize, string datfilename)
{
	strcpy(m_filename,filename);
	memcpy(m_hash,hash,sizeof(m_hash));
	m_hash[40] = '\0';
	strcpy(m_datfilename,datfilename.c_str());
	m_size = filesize;
	m_time = CTime::GetCurrentTime();

}

DatfileRecord DatfileRecord::operator =(DatfileRecord Record)
{
	this->m_size = Record.m_size;
	this->m_time = Record.m_time;
	strcpy(this->m_datfilename, Record.m_datfilename);
	strcpy(this->m_filename, Record.m_filename);
	strcpy(this->m_hash, Record.m_hash);

	return *this;
}