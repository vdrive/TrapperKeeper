#pragma once
#include "..\tkcom\object.h"

class DB2000 :	public DBPrimitive  //derive from DBPrimitive to allow nested tables.
{
public:
	DB2000(void);
	~DB2000(void);
};
