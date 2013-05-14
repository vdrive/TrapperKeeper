#include "StdAfx.h"
#include "comsubscriber.h"

ComSubscriber::ComSubscriber(UINT op_code,UINT from_app)
{
	m_op_code=op_code;
	m_from_app=from_app;
	m_ref_count=1;
}

ComSubscriber::~ComSubscriber(void)
{
}

void ComSubscriber::AddRef(void)
{
	m_ref_count++;
}

void ComSubscriber::DeRef(void)
{
	m_ref_count--;
}

bool ComSubscriber::IsEmpty(void)
{
	if(m_ref_count<=0)
		return true;
	else return false;
}
