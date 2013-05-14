#pragma once
#include "Hash20Byte.h"
#include "TorrentPiece.h"
#include <afx.h>
#include <string>
#include <set>
using namespace std;

//
//
//
class Torrent
{
public:
	Torrent(void);
	Torrent(unsigned int nTorrentID, const char *pFilename, const char *pInfoHash,
		unsigned int nPieceLength, unsigned int nTotalLength, const char *pTime);
	Torrent(const unsigned char *pInfoHash);
	Torrent(const Hash20Byte &rInfoHash);
	Torrent(const Torrent &rTorrent);
	virtual ~Torrent(void);

	inline unsigned int GetTorrentID(void) const { return m_nTorrentID; }

	inline const char *GetFilename(void) const { return m_filename.c_str(); }

	inline void SetInfoHash(string infohash) { m_infohash.FromHexString( infohash.c_str() ); }
	inline const char *GetInfoHashPrintable(char *pBuf) const { return m_infohash.ToPrintableCharPtr( pBuf ); }
	inline const unsigned char *GetInfoHashBytePtr(void) const { return m_infohash.ToBytePtr(); }

	inline unsigned int GetPieceLength(void) const { return m_nPieceLength; }
	inline unsigned int GetTotalLength(void) const { return m_nTotalLength; }

	inline bool AddSeededPiece(int nPiece, int nTimeInterval) { return m_sSeededPieces.insert( TorrentPiece( nPiece, nTimeInterval ) ).second; }
	inline bool AddSeedlessPiece(int nPiece) { return m_sSeedlessPieces.insert( nPiece ).second; }

	inline unsigned int GetBitsInBitfield(void) const { return m_nBitsInBitfield; }
	inline unsigned int GetBytesInBitfield(void) const { return m_nBytesInBitfield; }
	inline unsigned int GetBitsInBitfieldFinalByte(void) const { return m_nBitsInBitfieldFinalByte; }

	inline void SetActivationTime(void) { m_activationTime = CTime::GetCurrentTime(); }
	inline CTime GetActivationTime(void) const { return m_activationTime; }

	Torrent &operator=(const Torrent &rRight);
	inline bool operator==(const Torrent &rRight) const { return (m_infohash ==  rRight.m_infohash); }
	inline bool operator!=(const Torrent &rRight) const { return (m_infohash !=  rRight.m_infohash); }
	friend inline bool operator<(const Torrent &rLeft, const Torrent &rRight) { return rLeft.m_infohash < rRight.m_infohash; }
	friend inline bool operator>(const Torrent &rLeft, const Torrent &rRight) { return rLeft.m_infohash > rRight.m_infohash; }

	inline bool IsTorrent(void) const { return ( m_nTorrentID != 0 && !m_infohash.IsZero() &&
		m_nPieceLength != 0 && m_nTotalLength != 0 && !m_sSeedlessPieces.empty() && !m_sSeededPieces.empty() &&
		m_nBitsInBitfield != 0 && m_nBytesInBitfield != 0 && m_activationTime != CTime( 1980, 01, 01, 00, 00, 00 ) ); }

	void Clear(void);

public:
	unsigned int		m_nTorrentID;

	string				m_filename;
	Hash20Byte			m_infohash;

	unsigned int		m_nPieceLength;
	unsigned int		m_nTotalLength;

	set<unsigned int>	m_sSeedlessPieces;
	set<TorrentPiece>	m_sSeededPieces;

	unsigned int		m_nBitsInBitfield;
	unsigned int		m_nBytesInBitfield;
	unsigned int		m_nBitsInBitfieldFinalByte;

	CTime				m_activationTime;

};
