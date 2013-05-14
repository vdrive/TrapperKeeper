#pragma once
#include <string>
using namespace std;

//
class TorrentEntry
{
public:

	TorrentEntry(void)
		: m_nTorrentID(0), m_nLength(0), m_bZeroSeedInflate(false), m_bGenerate(false), m_bActive(false)
		, m_nBaseSeeds(0), m_nSeedMultiplier(1), m_nBasePeers(0), m_nPeerMultiplier(1)
		, m_nBaseCompletes(0), m_nCompleteMultiplier(1), m_bEditable(true)
	{
		memset( m_aCreationDate, 0, 20 );
		memset( m_aActivationDate, 0, 20 );
	}

	~TorrentEntry(void)	{}

public:
	unsigned int m_nTorrentID;
	string m_filename;
	string m_hash;
	char m_aCreationDate[20];
	char m_aActivationDate[20];
	string m_comment;
	unsigned int m_nLength;

	bool m_bZeroSeedInflate;
	bool m_bGenerate;
	bool m_bActive;

	unsigned int m_nBaseSeeds;
	unsigned int m_nSeedMultiplier;
	unsigned int m_nBasePeers;
	unsigned int m_nPeerMultiplier;
	unsigned int m_nBaseCompletes;
	unsigned int m_nCompleteMultiplier;

	bool m_bEditable;
};
