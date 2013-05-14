#pragma once
#include "BEncoding.h"

#include <vector>
using namespace std;

class BList
	: public BEncoding
{
public:
	BList(void);
	virtual ~BList(void);

	void addItem(BEncoding * item);

	vector<BEncoding *> v_items;

	//void ReadSelf(CFile *file);
	void WriteEncodedData(CFile *pOutputFile);
};
