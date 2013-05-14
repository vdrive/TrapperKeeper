#include "StdAfx.h"
#include ".\torrentfiledata.h"

TorrentFileData::TorrentFileData()
{
	Clear();
}

TorrentFileData::~TorrentFileData(void)
{
}
	
//
///
//
void TorrentFileData::Clear()
{
	m_announce_URL.clear();
	m_comment.clear();
	m_created_by.clear();
	m_creation_date = 0;
	m_name.clear();
	m_piece_length = 0;
	v_announce_list.clear();
	v_files.clear();
	v_piece_hashes.clear();
	m_hash.clear();
}
	
//
//
//
int TorrentFileData::GetBufferLength()
{
	int len = 0;

	len += (int)m_announce_URL.size()+1;	//+1 for NULL
	len += (int)m_comment.size()+1;
	len += (int)m_created_by.size()+1;
	len += sizeof(m_creation_date);
	len += (int)m_name.size()+1;
	len += sizeof(m_piece_length);

	len += sizeof(unsigned int);	//number of announce list entries
	for(int i=0; i<v_announce_list.size(); i++)
	{
		len += (int)(v_announce_list[i].size())+1;
	}

	len += sizeof(unsigned int);
	for(i=0; i<v_piece_hashes.size(); i++)
	{
		len += (int)(v_piece_hashes[i].size())+1;
	}

	len += sizeof(unsigned int);
	for(i=0; i<v_files.size(); i++)
	{
		len += v_files[i].GetBufferLength();
	}

	len += (int)m_hash.size()+1;


	return len;	
}
	
//
//
//
int TorrentFileData::WriteToBuffer(char *buf)
{
	int i;

	int buf_length = GetBufferLength();

	memset(buf, 0, buf_length);

	char * ptr =  buf;

	strcpy(ptr, m_announce_URL.c_str());
	ptr+=m_announce_URL.size()+1;	//+1 for NULL

	strcpy(ptr, m_comment.c_str());
	ptr+=m_comment.size()+1;	//+1 for NULL

	strcpy(ptr, m_created_by.c_str());
	ptr+=m_created_by.size()+1;	//+1 for NULL

	*((unsigned int *)ptr)=m_creation_date;
	ptr+=sizeof(unsigned int);

	strcpy(ptr, m_name.c_str());
	ptr+=m_name.size()+1;	//+1 for NULL

	*((unsigned int *)ptr)=m_piece_length;
	ptr+=sizeof(unsigned int);

	*((unsigned int *)ptr)=v_announce_list.size();
	ptr+=sizeof(unsigned int);

	for(i=0; i<v_announce_list.size(); i++)
	{
		strcpy(ptr, v_announce_list[i].c_str());
		ptr+=v_announce_list[i].size()+1;	//+1 for NULL
	}
	
	*((unsigned int *)ptr)=v_piece_hashes.size();
	ptr+=sizeof(unsigned int);

	for(i=0; i<v_piece_hashes.size(); i++)
	{
		strcpy(ptr, v_piece_hashes[i].c_str());
		ptr+=v_piece_hashes[i].size()+1;	//+1 for NULL
	}

	*((unsigned int *)ptr)=v_files.size();
	ptr+=sizeof(unsigned int);

	for(i=0; i<v_files.size(); i++)
	{	
		ptr+=v_files[i].WriteToBuffer(ptr);
	}

	strcpy(ptr, m_hash.c_str());
	ptr+=m_hash.size()+1;

	return buf_length;
}

//
//
//
int TorrentFileData::ReadFromBuffer(char *buf)
{
	int i;

	Clear();

	char *ptr = buf;
	char *bufbegin = buf;
	
	m_announce_URL = ptr;
	ptr+=m_announce_URL.size()+1;

	m_comment = ptr;
	ptr+=m_comment.size()+1;

	m_created_by = ptr;
	ptr+=m_created_by.size()+1;
		
	m_creation_date=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

	m_name = ptr;
	ptr+=m_name.size()+1;

	m_piece_length=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

	int len=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

	for(i=0;i<len;i++)
	{
		v_announce_list.push_back(ptr);
		ptr+=v_announce_list[i].size()+1;
	}

	len=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

	for(i=0;i<len;i++)
	{
		v_piece_hashes.push_back(ptr);
		ptr+=v_piece_hashes[i].size()+1;
	}

	len=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

	for(i=0;i<len;i++)
	{
		FileInfo file;
//		ptr += file.ReadFromBuffer(ptr)
//		v_files.push_back(file);
	}

	m_hash = ptr;
	ptr+=m_hash.size()+1;

	return (int)(ptr-bufbegin);
}