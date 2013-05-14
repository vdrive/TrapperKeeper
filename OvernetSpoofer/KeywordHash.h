#pragma once
#include "..\DCMaster\ProjectKeywords.h"
#include "SupplyDataSet.h"

class KeywordHash
{
public:
	KeywordHash(void);
	~KeywordHash(void);
	void Clear();
	bool operator ==(KeywordHash &other);
	bool operator < (const KeywordHash&  other)const;
	void SetHash(byte* hash);
	void SetUserHash(byte* hash);

	byte m_hash[16];
	byte m_my_user_hash[16];
	unsigned int m_checksum;
	CString m_keyword;
	vector<vector<ProjectKeywords>::iterator> vp_project_keywords;
};

class KeywordHashHash : public hash_compare <KeywordHash>
{
public:
	using hash_compare<KeywordHash>::operator();
	size_t operator()(const KeywordHash& key)const
	{
		return key.m_checksum;
	}
};
