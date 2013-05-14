// SupplyKeywords.h

#ifndef SUPPLY_KEYWORDS_H
#define SUPPLY_KEYWORDS_H

#include "ProjectDataStructure.h"
#include "SupplyKeyword.h"

class SupplyKeywords : public ProjectDataStructure
{
public:
	// Public Member Functions
	SupplyKeywords();
	~SupplyKeywords();

	void Clear();
	int GetBufferLength();
	int WriteToBuffer(char *buf);
	int ReadFromBuffer(char *buf);

	// Public Data Members
	unsigned int m_supply_size_threshold;
	string m_search_string;
	vector<SupplyKeyword> v_killwords;
	vector<SupplyKeyword> v_keywords;
};

#endif // SUPPLY_KEYWORDS_H