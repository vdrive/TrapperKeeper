#pragma once
#include "uasyncsocket.h"

class SearchResult :
	public UAsyncSocket
{
public:
	SearchResult(void);
	~SearchResult(void);
};
