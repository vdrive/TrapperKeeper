// WSocketSendData.cpp

#include "stdafx.h"
#include "WSocketSendData.h"

//
//
//
WSocketSendData::WSocketSendData(void *data,unsigned int data_len)
{
	m_data_len=data_len;
	p_data=new unsigned char[m_data_len];
	memcpy(p_data,data,m_data_len);
}

//
//
//
WSocketSendData::~WSocketSendData()
{
	delete [] p_data;
}

//
//
//
unsigned int WSocketSendData::GetDataLength()
{
	return m_data_len;
}

//
//
//
unsigned char *WSocketSendData::GetDataBuffer()
{
	return p_data;
}