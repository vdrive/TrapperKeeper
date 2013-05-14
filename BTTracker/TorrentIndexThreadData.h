#pragma once
#include "Torrent.h"
#include <set>
using namespace std;

class TorrentIndexThreadData
{
public:

	TorrentIndexThreadData(void)
		: m_sTorrents(), m_bQuit(false)
	{
	}

	~TorrentIndexThreadData(void)
	{
	}

public:
	set<Torrent>	m_sTorrents;
	volatile bool	m_bQuit;
};
