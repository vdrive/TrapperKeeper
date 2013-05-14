#pragma once
#include "ProjectDataStructure.h"
#include "BTIP.h"

class BTIPList : public ProjectDataStructure
{
public:
	BTIPList(void);
	BTIPList(int id);
	~BTIPList(void);

	/*
	int GetID();
	void SetID(int id);
	*/
	vector<BTIP> GetIps();
	void AddIpList(vector<BTIP> ips);
	void AddIp(BTIP ip);
	void CalculateNumIPs();


	
	virtual void Clear();						// call this in the constructors of the derived classes
	virtual int GetBufferLength();
	virtual int WriteToBuffer(char *buf);
	virtual int ReadFromBuffer(char *buf);	// returns buffer length

	int m_num_ips;
	vector<BTIP> v_ips;



};
