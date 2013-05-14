#pragma once

//#include "SupplyEntry.h"
#include "ProjectDataStructure.h"

#define UPDATE_STRING "(update)"

class SupplyProject : public ProjectDataStructure
{
public:
	// Public Data Members
	string m_name;
//	bool m_uber_dist_enabled;
	unsigned int m_offset;
	unsigned int m_interval;
//	unsigned int m_calculate_supply_counter;
//	GUID m_query_guid;

//	unsigned int m_num_spoofs;

//	vector<SupplyEntry> v_spoof_entries;
//	vector<SupplyEntry> v_dist_entries;


	// Public Member Functions
	SupplyProject();
	~SupplyProject();
	void Clear();						// call this in the constructors of the derived classes
	int GetBufferLength();				// returns buffer length
	int WriteToBuffer(char *buf);
	int ReadFromBuffer(char *buf);

//	void DeleteSupplyFile();
//	bool GUIDIsNull();
};