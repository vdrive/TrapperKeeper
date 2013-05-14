#pragma once

#include "StdAfx.h"
#include "DBInterface.h"
#include "DataFile.h"
#include "BInteger.h"
#include "BString.h"

#define DEFAULT_CHUNK_SIZE		262144
#define HASH_SIZE				20


// BTInput inputs the information on the torrent file from the database.
//	This is the main data structure for the torrent file, and has fields
//	for the announcelist, piece hashes and all the other meta info of
//	the torrent file.
class BList;
class BTInput
{
public:
	
	BTInput(string torrentFilename, unsigned int nTorrentID);
	virtual ~BTInput(void);

	inline string GetTorrentFileName(void) { return m_torrentFilename; }
	inline unsigned int GetTorrentID(void) const { return m_nTorrentID; }

	inline void AddTrackerURL(BString *pTracker) { m_vBTrackers.push_back( pTracker ); }
	inline vector <BString *> GetTrackerURLs(void) { return m_vBTrackers; }
	inline size_t TrackerURLCount(void) { return m_vBTrackers.size(); }
	string GetPrimaryTrackerURL(void);

	void PutInfohash(const unsigned char *);
	inline unsigned char *GetInfohash(void) { return m_pInfohash; }
	string GetHashString(void);

	void SetPieceLength(int nPieceLength);
	inline int GetPieceLength(void) const { return m_pBPieceLength->signedValue; }
	inline BInteger *GetBPieceLength(void) const { return m_pBPieceLength; }

	inline const BString *GetBTorrentFilename(void) { return m_pBTorrentFilename; }

	void PutPieceHashes(string hashes);
	BString *GetBPieceHashes(void) { return m_pBPieceHashes; }

	void SetParentDataname(string parentDataname);
	inline BString *GetBParentDataname(void) { return m_pBParentDataname; }

	inline void AddDataFile(DataFile *pDataFile) { m_vDataFiles.push_back( pDataFile ); }
	inline vector<DataFile *> GetDataFilesVector(void) { return m_vDataFiles; }

	inline void SetTotalSize(unsigned int size) { m_nTotalDataSize = size; }
	inline unsigned int GetTotalSize(void) const { return m_nTotalDataSize; }

	void SetCreationTime(time_t time);
	inline BInteger *GetCreationTime(void) { return m_pBCreationTime; }

	inline void SetZeroSeedInflate(bool bVal) { m_bZeroSeedInflated = bVal; }
	inline bool NeedsZeroSeedInflation(void) const { return m_bZeroSeedInflated; }
	

protected:
	string				m_torrentFilename;
	unsigned int		m_nTorrentID;

	BString *			m_pBParentDataname;

	BString *			m_pBTorrentFilename;
	BInteger *			m_pBFilesize;
	BString *			m_pBTracker;

	BInteger *			m_pBPieceLength;

	BString *			m_pBPieceHashes;
	unsigned char *		m_pInfohash;
	unsigned int		m_nTotalDataSize;

	BInteger *			m_pBCreationTime;

	vector<DataFile *>	m_vDataFiles;
	vector<BString *>	m_vBTrackers;

	BList *				m_pBFiles;

	bool				m_bZeroSeedInflated;
};
