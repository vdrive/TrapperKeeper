#include "StdAfx.h"
#include "receivedmessage.h"

ReceivedMessage::ReceivedMessage(const char* source_ip,byte* data,UINT data_length)
{
	m_header=*(UINT*)data;
	m_msg.WriteBytes(data+4,data_length-4);
	m_source_ip=source_ip;
}

ReceivedMessage::~ReceivedMessage(void)
{
}

UINT ReceivedMessage::GetHeader(void)
{
	return m_header;
}

Buffer2000* ReceivedMessage::GetData()
{
	return &m_msg;
}

const char* ReceivedMessage::GetSource(void)
{
	return m_source_ip.c_str();
}
