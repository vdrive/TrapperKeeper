// TSocketSendData.cpp

#include "stdafx.h"
#include "TSocketSendData.h"

//
//
//
TSocketSendData::TSocketSendData(void *data,unsigned int data_len)
{
	m_data_len=data_len;
	p_data=new unsigned char[m_data_len];
	memcpy(p_data,data,m_data_len);
}

//
//
//
TSocketSendData::~TSocketSendData()
{
	delete [] p_data;
}

//
//
//
unsigned int TSocketSendData::GetDataLength()
{
	return m_data_len;
}

//
//
//
unsigned char *TSocketSendData::GetDataBuffer()
{
	return p_data;
}