#include "StdAfx.h"
#include "racksupernodes.h"

RackSuperNodes::RackSuperNodes(void)
{
	 m_rack_name = "NULL";
	 m_num_kazaa = DEFAULT_KAZAA_INSTANCES; //default number of kazaa to run
	 m_num_kazaa_running = 0;
	 m_ping_offset=0;
}

//
//
//
RackSuperNodes::~RackSuperNodes(void)
{
	v_supernodes.clear();
}

//
//
//
bool RackSuperNodes::operator ==(const RackSuperNodes& right)const
{
	if(strcmp(m_rack_name.c_str(),right.m_rack_name.c_str())==0)
		return true;
	return false;
}

//
//
//
bool RackSuperNodes::operator < (const RackSuperNodes& right)const
{
	int ret = 0;
	ret = strcmp(m_rack_name.c_str(),right.m_rack_name.c_str());
	if(ret < 0)
		return true;
	else
		return false;
}
