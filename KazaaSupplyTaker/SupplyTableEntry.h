// SupplyTableEntry.h

#pragma once

#include <string>
using namespace std;

class SupplyTableEntry
{
public:
	SupplyTableEntry();
	void Clear();

	unsigned int m_project_id;
	unsigned int m_user_id;
	unsigned int m_file_id;
};