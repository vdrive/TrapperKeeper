#pragma once
using namespace std;
//Each rack has all these properties
class RackInfo
{
public:
	RackInfo(void);
	~RackInfo(void);
	string ip;				//stores the ip address of rack
	string network;		//stores the network address
	unsigned int port;				//stores the port of the rack
	string cabinet;		//stores the cabinet of the rack
	string cabinet_switch;	//stores the name of the cabinet switch rack is on
	string rackname;		//stores the name of the rack
};
