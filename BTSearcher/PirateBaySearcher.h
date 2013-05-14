#pragma once
#include "websearcher.h"

class PirateBaySearcher :
	public WebSearcher
{
public:
	PirateBaySearcher(void);
	~PirateBaySearcher(void);

	void FormURLs();
	int GetFirstPage();
};
