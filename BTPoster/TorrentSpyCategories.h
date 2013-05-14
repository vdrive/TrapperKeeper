#pragma once

class TorrentSpyCategories
{
public:
	char *m_names;
	char *m_ids;
	TorrentSpyCategories(char *names, char*ids)
	{
		m_names = names;
		m_ids = ids;
	}

	~TorrentSpyCategories(void)
	{
	}
};
