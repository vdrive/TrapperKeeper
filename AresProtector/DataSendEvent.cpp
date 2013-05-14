#include "StdAfx.h"
#include "datasendevent.h"

DataSendEvent::DataSendEvent(UINT amount)
{
	m_time=CTime::GetCurrentTime();
	m_amount=amount;
}

DataSendEvent::~DataSendEvent(void)
{
}
