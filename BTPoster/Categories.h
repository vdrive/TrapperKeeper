#pragma once

class Categories
{
public:
	char *m_names;
	char *m_ids;
	Categories(char *names, char*ids)
	{
		m_names = names;
		m_ids = ids;
	}

	~Categories(void)
	{
	}
};
