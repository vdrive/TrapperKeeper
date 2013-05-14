#pragma once
#include "websearcher.h"

class MiniNovaSearcher :
	public WebSearcher
{
public:
	MiniNovaSearcher(void);
	~MiniNovaSearcher(void);

	void FormURLs();
};
