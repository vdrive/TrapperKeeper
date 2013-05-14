# pragma once

#include "BTInput.h"
#include "SHA1.h"
#include "DBInterface.h"

/* Torrent Creator class does most of the work in generating a torrent.
It generates the torrent file and writes it into the trapperkeeper directory.
It computes the infohash and outputs the infohash, piecehashes, and other 
output metadata of the torrent into the database.
*/

class BTTorrentGeneratorDll;
class TorrentCreator
{
// functions
public:
	TorrentCreator();
	~TorrentCreator();

	bool MakeTorrent(BTInput *pTorrent);

public:
	int GetDecoyData(UINT &rnDecoySize, const UINT nDataSize, UINT &rnDecoyOffset, UCHAR *pDataBuf);	

public:
	string BufToString(unsigned char *pBuf, unsigned int nLen);
	void WriteTorrentInfoFile(vector<DataFile *> vDatafiles);
	void WriteHashToDatabase(void);
	string GetHashString(unsigned char *pHash);
	size_t GetTorrents(vector <BTInput *> &rvTorrents);
	void SetPieceValues(void);
	void WriteOutputToFile(void);
	void WriteTorrentToDatabase(void);
	bool InitiateZeroSeedBoosting(void);

private:
	void ComputeInfohash(void);

// variables
private:
	BTTorrentGeneratorDll *		m_pParrent;

	CFile						m_tempOutputFile;
	CFile						m_decoyDataFile;

	BTInput *					m_pIPData;

	string						m_hashString;
	unsigned int				m_nLastAutoID;
	string						m_primaryTracker;

	vector<UCHAR *>				m_vPieceHashes;
	UCHAR *						m_pInfoBuffer;
	ULONG						m_nInfoBufferSize;
};
