// SupplyKeyword.h

#ifndef SUPPLY_KEYWORD_H
#define SUPPLY_KEYWORD_H

class SupplyKeyword
{
public:
	// Public Member Functions
	SupplyKeyword();
	SupplyKeyword(const SupplyKeyword &keyword);	// copy constructor
	~SupplyKeyword();
	void Clear();

	SupplyKeyword& operator=(const SupplyKeyword &keyword);

	int GetBufferLength();
	int WriteToBuffer(char *buf);
	int ReadFromBuffer(char *buf);

	// Public Data Members
	bool m_single;
	unsigned int m_track;
	string m_track_name;
	vector<char *> v_keywords;
};

#endif // SUPPLY_KEYWORD_H