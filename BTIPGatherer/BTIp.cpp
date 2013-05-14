#include "StdAfx.h"
#include ".\btip.h"

BTIP::BTIP(void)
{
}

BTIP::~BTIP(void)
{
}

void BTIP::Clear()
{

//	memcpy(&m_id[0],'\0',1);
//	memcpy(&m_ip[0],'\0',1);
	m_id[0] = NULL;
	m_ip[0] = NULL;
	m_port = 0;
	m_torrent_id = 0;
}

int BTIP::GetBufferLength()
{
	int len = 0;

	len += sizeof(m_id);
	len += (int)strlen(m_ip)+1;
	len += sizeof(int);
	len += sizeof(int);

	return len;	
}

int BTIP::WriteToBuffer(char * buf)
{

	int buf_length = GetBufferLength();

	memset(buf, 0, buf_length);

	char *ptr = buf;
	char *bufbegin = buf;

	strcpy(ptr, m_id);
	ptr+=strlen(m_id);
	*ptr = '\0';
	ptr+=1;	//+1 for NULL

	strcpy(ptr, m_ip);
	ptr+=strlen(m_ip);
	*ptr = '\0';	
	ptr+=1;	//+1 for NULL

	*((int *)ptr)=m_port;
	ptr+=sizeof(int);

	*((int *)ptr)=m_torrent_id;
	ptr+=sizeof(int);

	int len = (int)(ptr-bufbegin);

	return buf_length;


}

int BTIP::ReadFromBuffer(char * buf)
{

	Clear();

	char *ptr = buf;
	char *bufbegin = buf;


	strcpy(m_id, ptr);
	ptr+=strlen(m_id)+1;

	strcpy(m_ip, ptr);
	ptr+=strlen(m_ip)+1;

			
	m_port=*((int *)ptr);
	ptr+=sizeof(int);

	m_torrent_id=*((int *)ptr);
	ptr+=sizeof(int);

	return (int)(ptr-bufbegin);


}