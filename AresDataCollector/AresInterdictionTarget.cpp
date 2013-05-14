#include "StdAfx.h"
#include "aresinterdictiontarget.h"
#include "..\aresprotector\AresUtilityClass.h"

AresInterdictionTarget::AresInterdictionTarget(unsigned int ip, unsigned short port,unsigned int size, const char* hash)
{
	m_ip=ip;
	m_port=port;
	m_size=size;
	
	for(int j=0;j<20;j++){
		char ch1=hash[j*2];
		char ch2=hash[j*2+1];
		byte val1=AresUtilityClass::ConvertCharToInt(ch1);
		byte val2=AresUtilityClass::ConvertCharToInt(ch2);
		byte hash_val=((val1&0xf)<<4)|(val2&0xf);
		m_hash[j]=hash_val;
	}				
}

AresInterdictionTarget::~AresInterdictionTarget(void)
{
}
