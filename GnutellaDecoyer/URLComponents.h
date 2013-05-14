// URLComponents.h
#pragma once

#include <afxinet.h>

#define BUF_LEN 4096

class URLComponents
{
public:
	URLComponents();
	~URLComponents();
	
	URL_COMPONENTS m_data;
};