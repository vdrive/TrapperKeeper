#include "Torrent.h"
#include <math.h>

//
//
//
Torrent::Torrent(void)
: m_nTorrentID(0), m_filename(""), m_infohash(), m_nPieceLength(0)
, m_nTotalLength(0), m_sSeedlessPieces(), m_sSeededPieces()
, m_nBitsInBitfield(0), m_nBytesInBitfield(0), m_nBitsInBitfieldFinalByte(0)
{
}

//
//
//
Torrent::Torrent(unsigned int nTorrentID, const char *pFilename, const char *pInfoHash,
				 unsigned int nPieceLength, unsigned int nTotalLength, const char *pTime)
				 : m_nTorrentID(0), m_filename(""), m_infohash(), m_nPieceLength(0)
				 , m_nTotalLength(0), m_sSeedlessPieces(), m_sSeededPieces()
{
	if( pFilename == NULL || pInfoHash == NULL || pTime == NULL )
		return;

	m_nTorrentID = nTorrentID;
	m_filename = pFilename;
	m_infohash.FromHexString( pInfoHash );
	m_nPieceLength = nPieceLength;
	m_nTotalLength = nTotalLength;

	m_nBitsInBitfield = (unsigned int)ceil( nTotalLength / (double)nPieceLength );;
	m_nBytesInBitfield = (unsigned int)ceil( m_nBitsInBitfield / (double)8 );
	m_nBitsInBitfieldFinalByte = m_nBitsInBitfield % 8;
	if( m_nBitsInBitfieldFinalByte == 0 )
		m_nBitsInBitfieldFinalByte = 8;

	int nYear, nMonth, nDay, nHour, nMinute, nSecond;
	sscanf( pTime, "%4d%2d%2d%2d%2d%2d", &nYear, &nMonth, &nDay, &nHour, &nMinute, &nSecond );
	m_activationTime = CTime( nYear, nMonth, nDay, nHour, nMinute, nSecond );
}

//
//
//
Torrent::Torrent(const unsigned char *pInfoHash)
: m_nTorrentID(0), m_filename(""), m_infohash(), m_nPieceLength(0), m_nTotalLength(0)
, m_sSeedlessPieces(), m_sSeededPieces(), m_nBitsInBitfield(0), m_nBytesInBitfield(0)
, m_nBitsInBitfieldFinalByte(0), m_activationTime(1980, 01, 01, 00, 00, 00)
{
	if( pInfoHash == NULL )
		return;

	m_infohash.FromEscapedHexString( (const char *)pInfoHash );
}

//
//
//
Torrent::Torrent(const Hash20Byte &rInfoHash)
: m_nTorrentID(0), m_filename(""), m_infohash(rInfoHash), m_nPieceLength(0)
, m_nTotalLength(0), m_sSeedlessPieces(), m_sSeededPieces()
, m_nBitsInBitfield(0), m_nBytesInBitfield(0), m_nBitsInBitfieldFinalByte(0)
, m_activationTime(1980, 01, 01, 00, 00, 00)
{
}

//
//
//
Torrent::Torrent(const Torrent &rTorrent)
: m_nTorrentID(rTorrent.m_nTorrentID), m_filename(rTorrent.m_filename), m_infohash(rTorrent.m_infohash)
, m_nPieceLength(rTorrent.m_nPieceLength), m_nTotalLength(rTorrent.m_nTotalLength)
, m_sSeedlessPieces(rTorrent.m_sSeedlessPieces), m_sSeededPieces(rTorrent.m_sSeededPieces)
, m_nBitsInBitfield(rTorrent.m_nBitsInBitfield), m_nBytesInBitfield(rTorrent.m_nBytesInBitfield)
, m_nBitsInBitfieldFinalByte(rTorrent.m_nBitsInBitfieldFinalByte), m_activationTime(rTorrent.m_activationTime)
{
}

//
//
//
Torrent::~Torrent(void)
{
}

//
//
//
Torrent &Torrent::operator=(const Torrent &rRight)
{
	m_nTorrentID = rRight.m_nTorrentID;
	m_filename = rRight.m_filename;
	m_infohash = rRight.m_infohash;

	m_nPieceLength = rRight.m_nPieceLength;
	m_nTotalLength = rRight.m_nTotalLength;
	m_sSeedlessPieces = rRight.m_sSeedlessPieces;
	m_sSeededPieces = rRight.m_sSeededPieces;

	m_nBitsInBitfield = rRight.m_nBitsInBitfield;
	m_nBytesInBitfield = rRight.m_nBytesInBitfield;
	m_nBitsInBitfieldFinalByte = rRight.m_nBitsInBitfieldFinalByte;

	m_activationTime = rRight.m_activationTime;

	return *this;
}

//
//
//
void Torrent::Clear(void)
{
	m_nTorrentID = 0;
	m_filename = "";
	m_infohash.ZeroHash();
	m_nPieceLength = 0;
	m_nTotalLength = 0;
	m_sSeedlessPieces.clear();
	m_sSeededPieces.clear();

	m_nBitsInBitfield = 0;
	m_nBytesInBitfield = 0;
	m_nBitsInBitfieldFinalByte = 0;

	m_activationTime = CTime( 1980, 01, 01, 00, 00, 00 );
}
