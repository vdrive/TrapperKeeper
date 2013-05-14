#pragma once

class SpoofCollector
{
public:
	SpoofCollector(void);
	~SpoofCollector(void);
	
	//add spoof count
	void AddSpoof(int add_spoof);
	void AddDecoy(int add_decoy);	//adds decoys
	void AddRequest(int add_request);	//add requests

	string m_project;	//project name
	int m_spoof_count;		//spoof counter
	int m_decoy_count;		//decoys counter
	int m_requests_count;	//requests counter
};
