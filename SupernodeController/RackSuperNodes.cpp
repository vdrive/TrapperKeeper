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

//
//
//
void RackSuperNodes::ReplaceSupernode(IPAndPort& old_ip, IPAndPort& new_ip)
{
	bool found = false;
	for(UINT i=0; i<v_supernodes.size(); i++)
	{
		if(v_supernodes[i] == old_ip)
		{
			found = true;
			v_supernodes[i] = new_ip;
			break;
		}
	}
	if(!found)
		v_supernodes.push_back(new_ip);
}