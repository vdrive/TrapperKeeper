#pragma once
#include "WebSearcher.h"

class IsoHuntSearcher :
	public WebSearcher
{
public:
	IsoHuntSearcher(void);
	~IsoHuntSearcher(void);

	void FormURLs();
};
