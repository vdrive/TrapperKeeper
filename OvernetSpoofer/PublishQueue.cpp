#include "StdAfx.h"
#include ".\publishqueue.h"

PublishQueue::PublishQueue(void)
{
	Clear();
}

//
//
//
PublishQueue::~PublishQueue(void)
{
}

//
//
//
bool PublishQueue::operator < (const PublishQueue& other)const
{
	if(this->m_ip < other.m_ip)
		return true;
	else
		return false;
}

//
//
//
bool PublishQueue::operator ==(PublishQueue &other)
{
	return (this->m_ip == other.m_ip);
}

//
//
//
void PublishQueue::Clear()
{
	m_ip=m_port=0;
	v_to_be_published.clear();
}

//
//
//
MetaData PublishQueue::PublishAckReceived()
{
	if(v_to_be_published.size()==0)
	{
		MetaData decoy;
		return decoy;
	}
	MetaData data = v_to_be_published[0];
	v_to_be_published.erase(v_to_be_published.begin());	
	return data;
}

//
//
//
MetaData& PublishQueue::ReturnCurrentMetaData()
{
	if(v_to_be_published.size()==0)
	{
		MetaData decoy;
		return decoy;
	}
	return v_to_be_published[0];
}