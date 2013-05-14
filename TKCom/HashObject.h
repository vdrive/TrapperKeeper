#pragma once
#include "..\tkcom\object.h"

#define HASHTABLESIZE 5000

class HashObject : public Object
{
public:
	HashObject(void);
	~HashObject(void);
	virtual UINT GetHashRef(){return 0;}
};
