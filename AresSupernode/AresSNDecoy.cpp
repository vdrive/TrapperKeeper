#include "StdAfx.h"
#include "aressndecoy.h"
#include "..\AresProtector\AresUtilityClass.h"

AresSNDecoy::AresSNDecoy(const char* hash, UINT size)
{
	//m_hash_sum=0;
	for(int j=0;j<20;j++){
		char ch1=hash[j*2];
		char ch2=hash[j*2+1];
		byte val1=AresUtilityClass::ConvertCharToInt(ch1);
		byte val2=AresUtilityClass::ConvertCharToInt(ch2);
		byte hash_val=((val1&0xf)<<4)|(val2&0xf);
		m_hash[j]=hash_val;
		//m_hash_sum+=hash_val;
	}				
	m_size=size;
}

AresSNDecoy::~AresSNDecoy(void)
{
}

UINT AresSNDecoy::CalcDiff(byte* hash)
{
	UINT dif=0;
	for(int i=0;i<20;i++){
		dif+=abs((int)hash[i]-(int)m_hash[i]);
	}
	return dif;
}
