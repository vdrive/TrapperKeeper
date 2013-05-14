#pragma once
#include "dbprimitive.h"

class DBstring :	public DBPrimitive
{
	string m_value;
public:
	DBstring(void);
	~DBstring(void);

	const char* operator(LPCSTR)(DBstring &me){
		return m_value.c_str();
	}		
};
