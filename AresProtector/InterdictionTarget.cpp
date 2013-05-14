#include "StdAfx.h"
#include "interdictiontarget.h"
#include "AresUtilityClass.h"

InterdictionTarget::InterdictionTarget(unsigned int ip,unsigned short port, unsigned int size, const char* hash_str,byte* hash_byte)
{
	m_create_time=CTime::GetCurrentTime();
	m_fail_count=0;
	m_ip=ip;
	m_port=port;
	m_size=size;
	
	if(hash_byte!=NULL){
		memcpy(m_hash,hash_byte,20);
	}
	else if(hash_str!=NULL){
		AresUtilityClass::EncodeHashFromString(m_hash,20,hash_str);
	}

	mb_banned=false;
}

InterdictionTarget::InterdictionTarget(const char* ip,unsigned short port, unsigned int size, const char* hash_str,byte* hash_byte)
{
	m_create_time=CTime::GetCurrentTime();
	m_fail_count=0;
	unsigned int ip1,ip2,ip3,ip4;
	sscanf(ip,"%u.%u.%u.%u",&ip1,&ip2,&ip3,&ip4);
	m_ip=(ip1<<0)|(ip2<<8)|(ip3<<16)|(ip4<<24);  //little endian

	m_port=port;
	m_size=size;

	if(hash_byte!=NULL){
		memcpy(m_hash,hash_byte,20);
	}
	else if(hash_str!=NULL){
		AresUtilityClass::EncodeHashFromString(m_hash,20,hash_str);
	}

	mb_banned=false;
}

InterdictionTarget::~InterdictionTarget(void)
{
}
