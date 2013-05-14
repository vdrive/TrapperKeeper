#ifndef HTTP_HEADERS_H
#define HTTP_HEADERS_H

#include "stdafx.h"

class GnutellaHTTPHeaders
{
public:
	GnutellaHTTPHeaders();
	~GnutellaHTTPHeaders();
	void SetValues(CString p, CString v);
	CString getParam();
	CString getValue();
private:
	CString param;
	CString value;
	CString filename;
};

#endif // HTTP_HEADERS_H