#pragma once
#include <string>
#include "BEncoding.h"

#include "BString.h"

using namespace std;
class BInteger;
class BList;
class BDictionary;

class BDictionaryItem :	public BEncoding
{
public:
	BDictionaryItem(void);
	virtual ~BDictionaryItem(void);
//	BDictionaryItem(const BDictionaryItem &item);
//	BDictionaryItem &operator=(const BDictionaryItem &str);

	BString m_key;

	void addDictionaryItem(BDictionary * item, std::string key);
	void addListItem(BList * item, std::string  key);
	void addIntegerItem(BInteger * item, std::string  key);
	void addStringItem(BString * item, std::string  key);
	
	//void ReadSelf(CFile *file);
	void WriteEncodedData(CFile *pOutputFile);

	BString *GetBString();
	BInteger *GetBInteger();
	BList *GetBList();
	BDictionary *GetBDictionary();
	std::string GetKeyString();

private:
	BString *p_bstring;
	BInteger *p_binteger;
	BList *p_blist;
	BDictionary *p_bdictionary;
};
