#include "StdAfx.h"
#include "BTInput.h"
#include "TinySQL.h"
#include "BList.h"

//
//
//
BTInput::BTInput(string torrentFilename, unsigned int nTorrentID)
: m_torrentFilename(""), m_nTorrentID(0), m_pBParentDataname(NULL), m_pBTorrentFilename(NULL)
, m_pBFilesize(NULL), m_pBTracker(NULL), m_pBPieceLength(NULL), m_pBPieceHashes(NULL)
, m_pInfohash(NULL), m_nTotalDataSize(0), m_pBCreationTime(NULL), m_vDataFiles()
, m_vBTrackers(), m_pBFiles(NULL), m_bZeroSeedInflated(false)
{
	m_torrentFilename = torrentFilename;
	m_pBTorrentFilename = new BString(torrentFilename);

	m_nTorrentID = nTorrentID;
}

//
//
//
BTInput::~BTInput(void)
{
	if( m_pBParentDataname != NULL )
	{
		delete m_pBParentDataname;
		m_pBParentDataname = NULL;
	}

	if( m_pBTorrentFilename != NULL )
	{
		delete m_pBTorrentFilename;
		m_pBTorrentFilename = NULL;
	}

	if( m_pBFilesize != NULL )
	{
		delete m_pBFilesize;
		m_pBFilesize = NULL;
	}

	if( m_pBTracker != NULL )
	{
		delete m_pBTracker;
		m_pBTracker = NULL;
	}

	if( m_pBPieceLength != NULL )
	{
		delete m_pBPieceLength;
		m_pBPieceLength = NULL;
	}

	if( m_pBPieceHashes != NULL )
	{
		delete m_pBPieceHashes;
		m_pBPieceHashes = NULL;
	}

	if( m_pInfohash != NULL )
	{
		delete m_pInfohash;
		m_pInfohash = NULL;
	}

	if( m_pBCreationTime != NULL )
	{
		delete m_pBCreationTime;
		m_pBCreationTime = NULL;
	}

	if( m_pBFiles != NULL )
	{
		delete m_pBFiles;
		m_pBFiles = NULL;
	}

	for(vector <DataFile *>::iterator v_iter = m_vDataFiles.begin(); v_iter != m_vDataFiles.end(); v_iter++)
	{
		delete *v_iter;
	}
	m_vDataFiles.clear();

	for(vector <BString *>::iterator v_iter = m_vBTrackers.begin(); v_iter != m_vBTrackers.end(); v_iter++)
	{
		delete *v_iter;
	}
	m_vBTrackers.clear();
}

//
//
//
string BTInput::GetPrimaryTrackerURL(void)
{
	return string( (char *)(m_vBTrackers.at(0)->Buf()) );
}

//
//
//
void BTInput::PutInfohash(const unsigned char *buf)
{
	if( m_pInfohash != NULL )
		delete m_pInfohash;

	m_pInfohash = new UCHAR[ 20 ];
	memcpy( m_pInfohash, buf, 20 );
}

//
//
//
string BTInput::GetHashString(void)
{
	char aRet[41];

	for( int i = 0; i < 40; i += 2 )
		sprintf( aRet + i, "%02X", m_pInfohash[i/2] );

	return string(aRet);
}

//
//
//
void BTInput::SetPieceLength(int nPieceLength)
{
	if( m_pBPieceLength != NULL )
		delete m_pBPieceLength;

	m_pBPieceLength = new BInteger( nPieceLength );
}

//
//
//
void BTInput::PutPieceHashes(string hashes)
{
	if( m_pBPieceHashes != NULL )
		delete m_pBPieceHashes;

	m_pBPieceHashes = new BString(hashes);
}

//
//
//
void BTInput::SetParentDataname(string parentDataname)
{
	if( m_pBParentDataname != NULL )
		delete m_pBParentDataname;

	m_pBParentDataname = new BString( parentDataname );
}

//
//
//
void BTInput::SetCreationTime(time_t time)
{
	if( m_pBCreationTime != NULL )
		delete m_pBCreationTime;

	m_pBCreationTime = new BInteger( (int)time );
}
