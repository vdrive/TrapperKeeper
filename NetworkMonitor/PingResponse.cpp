#include "StdAfx.h"
#include "pingresponse.h"

PingResponse::PingResponse(bool b_tk_on)
{
	mb_tk_on=b_tk_on;
	m_response_time=CTime::GetCurrentTime();
}

PingResponse::~PingResponse(void)
{
}
