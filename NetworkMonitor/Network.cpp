#include "StdAfx.h"
#include "network.h"
#include "Rack.h"

Network::Network(const char* name)
{
	m_name=name;
}

Network::~Network(void)
{
}
int Network::GetTotalRacks(void)
{
	return (int)mv_racks.Size();
}

int Network::GetAliveRacks(void)
{
	int count=0;
	for(int i=0;i<(int)mv_racks.Size();i++){
		Rack *rack=(Rack*)mv_racks.Get(i);
		if(rack->IsAlive())
			count++;
	}
	return count;
}

int Network::GetCrashedRacks(void)
{
	int count=0;
	for(int i=0;i<(int)mv_racks.Size();i++){
		Rack *rack=(Rack*)mv_racks.Get(i);
		if(rack->IsAlive() && !rack->IsTKOn())
			count++;
	}
	return count;
}

bool Network::HasRack(const char* ip)
{
	for(int i=0;i<(int)mv_racks.Size();i++){
		Rack *rack=(Rack*)mv_racks.Get(i);
		if(stricmp(rack->m_ip.c_str(),ip)==0)
			return true;
	}
	return false;
}
