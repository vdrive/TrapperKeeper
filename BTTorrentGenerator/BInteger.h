#pragma once

#include <stdio.h>
#include <string>
#include "BEncoding.h"

class BInteger :
	public BEncoding
{
public:
	BInteger(void);
	virtual ~BInteger(void);

	BInteger(int);
	void WriteEncodedData(CFile *pOutputFile);

	unsigned int m_value;
	int signedValue;

};
