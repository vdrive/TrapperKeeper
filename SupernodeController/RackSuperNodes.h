#include "IPAndPort.h"
#pragma once

class RackSuperNodes
{
public:
	RackSuperNodes(void);
	~RackSuperNodes(void);
	bool operator ==(const RackSuperNodes& right)const;
	bool operator < (const RackSuperNodes&  right)const;
	void ReplaceSupernode(IPAndPort& old_ip, IPAndPort& new_ip);

	vector<IPAndPort> v_supernodes;
	string m_rack_name;
	UINT m_num_kazaa;			//maximum number of kazaa can run
	UINT m_num_kazaa_running;	//current number of kazaa running
	int m_ping_offset;
};
