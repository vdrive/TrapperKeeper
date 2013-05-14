#include "StdAfx.h"
#include "rack.h"

//
//
// 

Rack::Rack()
{
	
}
//
//
//
Rack::Rack(string rack_name)
{
	// init value:
	m_rack_name = rack_name;
	m_last_ping_time = CTime::GetCurrentTime();
}

//
//
//
Rack::~Rack(void)
{
}
