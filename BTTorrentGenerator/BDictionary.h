#pragma once
#include "StdAfx.h"
#include "BEncoding.h"
#include "BDictionaryItem.h"

class BDictionary
	:	public BEncoding
{
public:
	BDictionary(void);
	virtual ~BDictionary(void);

	void addItem(BDictionaryItem *item);
	void WriteEncodedData(CFile *pOutputFile);
	void WritePartialEncodedData(CFile *pOutputFile);

	vector<BDictionaryItem *> v_items;

	//void ReadSelf(CFile *file);
};
