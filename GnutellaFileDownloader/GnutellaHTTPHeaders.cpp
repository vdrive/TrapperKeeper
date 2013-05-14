#include "GnutellaHTTPHeaders.h"

GnutellaHTTPHeaders::GnutellaHTTPHeaders()
{
	param = "";
	value = "";
	filename = "";
}

GnutellaHTTPHeaders::~GnutellaHTTPHeaders()
{
	value.Empty();
	param.Empty();
	filename.Empty();
}

void GnutellaHTTPHeaders::SetValues(CString p, CString v)
{
	param = p;
	value = v;
	if(param == "Content-Disposition")
	{
		filename = value;
	}
}

CString GnutellaHTTPHeaders::getParam() 
{
	return param;
}
CString GnutellaHTTPHeaders::getValue() 
{
	return value;
}