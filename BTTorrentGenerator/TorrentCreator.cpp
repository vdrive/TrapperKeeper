#include "StdAfx.h"
#include "TorrentCreator.h"
#include "BDictionary.h"
#include "BList.h"
#include "TinySQL.h"

//
//
//
TorrentCreator::TorrentCreator()
: m_pIPData(NULL), m_nLastAutoID(0), m_pInfoBuffer(NULL), m_nInfoBufferSize(0)
{
	seedRand();

	// Open the temp buffer file
	if( !m_tempOutputFile.Open( "\\BTTorrentGenerator\\OutputData.tmp", CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyWrite ) )
	{
		TRACE( "Error opening \"\\BTTorrentGenerator\\OutputData.tmp\" temp buffer file\n" );
		MessageBox( NULL, "Error opening\n\"c:\\BTTorrentGenerator\\OutputData.tmp\"\ntemp buffer file",
			"No Temp File", MB_ICONEXCLAMATION | MB_OK );
		AfxAbort();
		return;
	}

	// Open the data file that is used to generate hashes, if it is not open already
	if( !m_decoyDataFile.Open( "\\syncher\\rcv\\plug-ins\\BTTorrentGenerator\\TorrentDecoyFile", CFile::modeRead | CFile::shareDenyWrite ) )
	{
		TRACE( "Error opening input decoy data file\n" );
		MessageBox( NULL, "Could not open decoy data file", "No Data File", MB_ICONEXCLAMATION | MB_OK );
		AfxAbort();
		return;
	}
}

//
//
//
TorrentCreator::~TorrentCreator()
{
	// Close and delete the temp buffer file
	if( m_tempOutputFile.m_hFile != CFile::hFileNull )
		m_tempOutputFile.Close();
	CFile::Remove( "c:\\BTTorrentGenerator\\OutputData.tmp" );

	// Close the decoy data file
	if( m_decoyDataFile.m_hFile != CFile::hFileNull )
		m_decoyDataFile.Close();

	for( vector<UCHAR *>::iterator v_iter = m_vPieceHashes.begin(); v_iter != m_vPieceHashes.end(); v_iter++ )
	{
		delete (*v_iter);
	}
	m_vPieceHashes.clear();

	if( m_pInfoBuffer != NULL )
	{
		delete m_pInfoBuffer;
		m_pInfoBuffer = NULL;
	}
}

/* Generates the piece hashes and the info hash for the torrent */
bool TorrentCreator::MakeTorrent(BTInput *pTorrent)
{
	// IF no torrent info exists, return
	if( pTorrent == NULL )
		return false;

	// Set the local to the torrent info
	m_pIPData = pTorrent;

	// Clear any piece hashes that we have
	if( !m_vPieceHashes.empty() )
	{
		for( int i = 0; i < (int)m_vPieceHashes.size(); i++ )
			delete m_vPieceHashes[i];

		m_vPieceHashes.clear();
	}

	// Seek the decoy file to the beginning
	m_decoyDataFile.SeekToBegin();

	// Make a new data buffer for the data of a single chunk of the "file data"
	UCHAR *pDataBuf = new UCHAR[ m_pIPData->GetPieceLength() ];

	// The datafiles are already sorted in the ascending order in which they must appear in the torrent file
	vector<DataFile *> vDatafiles = m_pIPData->GetDataFilesVector();

	// Calculate the total data size of the torrent by adding the size of each file in the torrent
	unsigned int nTotalDataSize = 0;
	for( vector<DataFile *>::iterator v_iter = vDatafiles.begin(); v_iter != vDatafiles.end(); v_iter++ )
	{
		nTotalDataSize += (*v_iter)->GetFileLength();
	}
	m_pIPData->SetTotalSize( nTotalDataSize );

	// Create variables for navigating the decoy data file
	UINT nDecoyFileSize = (UINT)m_decoyDataFile.GetLength();
	UINT nDecoyFileOffset = 0;
	UINT nDataRead = 0;
	UINT nPieceSize = m_pIPData->GetPieceLength();

	// While there is still more data to process
	while( nTotalDataSize > 0 )
	{
		//
		if( nTotalDataSize >= nPieceSize )
		{
			nDataRead = GetDecoyData(nDecoyFileSize, nPieceSize, nDecoyFileOffset, pDataBuf);
		}
		else
		{
			nDataRead = GetDecoyData(nDecoyFileSize, nTotalDataSize, nDecoyFileOffset, pDataBuf);
		}
		nTotalDataSize -= nDataRead;
		SHA1 sha1;
		sha1.Update(pDataBuf, nDataRead);
		sha1.Final();
		UCHAR *pSHA1Hash = new UCHAR[ 20 ];
		sha1.GetHash( pSHA1Hash );
		m_vPieceHashes.push_back( pSHA1Hash );
	}

	WriteTorrentInfoFile(vDatafiles);
	ComputeInfohash();

	delete pDataBuf;

	// Set the creation time
	time_t utc_time;
	time(&utc_time);
	//unsigned int cur_time = (unsigned int)utc_time;
	CTime now = CTime(utc_time);
	char time[15];
	sprintf(time, "%04u%02u%02u%02u%02u%02u", now.GetYear(), now.GetMonth(), now.GetDay(),
		now.GetHour(), now.GetMinute(), now.GetSecond());
	m_pIPData->SetCreationTime(utc_time);

	return true;
}

int TorrentCreator::GetDecoyData(UINT &rnDecoySize, const UINT nDataSize, UINT &rnDecoyOffset, UCHAR *pDataBuf)
{
	UINT nCurrentOffset = (UINT)m_decoyDataFile.GetPosition();
	UINT nDataRead = m_decoyDataFile.Read( pDataBuf, nDataSize );

	if( nDataRead < nDataSize )
	{
		rnDecoySize--;
		rnDecoyOffset++;

		if( rnDecoySize == 0 )
		{
			rnDecoySize = (UINT)m_decoyDataFile.GetLength();
			rnDecoyOffset = 0;
		}

		m_decoyDataFile.Seek( rnDecoyOffset, 0 );
		// IF any data could be read, try to fill the rest of the buffer with data starting from the
		//  start (plus offset) of the file
		if( nDataRead > 0 )
		{
			int nStillToRead = nDataSize - nDataRead;
			UCHAR *pBuf = new UCHAR[ nStillToRead ];
			int nData = GetDecoyData( rnDecoySize, nStillToRead, rnDecoyOffset, pBuf );
			if( nData != nStillToRead )
			{
				TRACE("ERROR reading decoy data");
				return 0;
			}
			for( int i = 0; i < nStillToRead; i++ )
			{
				pDataBuf[nDataRead+i] = pBuf[i];
			}
			delete pBuf;
			nDataRead += nStillToRead;
		}
		// ELSE IF no data could be read, fill the entire buffer from the start (plus offset) of the
		//  decoy data file
		else if( nDataRead == 0 )
		{
			nDataRead = GetDecoyData(rnDecoySize, nDataSize, rnDecoyOffset, pDataBuf);
		}
	}

	return nDataRead;
}


void TorrentCreator::ComputeInfohash(void)
{
	m_tempOutputFile.Flush();
	UINT nFileSize = (UINT)m_tempOutputFile.GetLength();
	m_tempOutputFile.SeekToBegin();

	if( m_pInfoBuffer != NULL )
	{
		delete m_pInfoBuffer;
	}
	m_pInfoBuffer = new UCHAR[ nFileSize ];
	m_nInfoBufferSize = nFileSize;

	m_tempOutputFile.Read( m_pInfoBuffer, nFileSize );

	SHA1 sha1;
	sha1.Update(m_pInfoBuffer, nFileSize);
	sha1.Final();

	UCHAR aSHA1Hash[20];
	sha1.GetHash( aSHA1Hash );
	m_pIPData->PutInfohash( aSHA1Hash );
}

/* Set 1% to 20% random pieces to not seed. The pieces are written
into the DB table bt_generator_seedless_pieces. They are read by the
BitTorrentClient. If the pieces are already set in the DB, we do
not compute new seedless pieces, for we want all the BitTorrentClients to
seed the same 80% to 99% pieces.
*/
void TorrentCreator::SetPieceValues(void)
{
	unsigned int torrent_id = m_pIPData->GetTorrentID();

	TinySQL db;
	char aQuery[1024];

	sprintf( aQuery, "SELECT piece_num FROM bt_generator_seedless_pieces WHERE torrent_id = %u", torrent_id );
	if( !db.Init( IP_BITTORRENT_DATABASE, "onsystems", "ebertsux37", "bittorrent_data", 3306 ) ||
		!db.Query( aQuery, true ) )
	{
		return;
	}

    // IF any rows are returned, the pieces have already have been set for this torrent
	if( db.m_nRows > 0 )
		return;

	// Query for the time intervals
	sprintf( aQuery, "SELECT minutes FROM bt_generator_time_intervals WHERE min_file_size < %u AND %u <= max_file_size ORDER BY minutes ASC",
		m_pIPData->GetTotalSize(), m_pIPData->GetTotalSize() );
	if( !db.Query( aQuery, true ) )
	{
		return;
	}

	int nTotalPieces = (int)m_vPieceHashes.size();

	int nIntervals = (int)db.m_nRows + 1;
	vector<int> vIntervalValues;
	vIntervalValues.push_back( 0 );
	for( int i = 0; i < (int)db.m_nRows; i++ )
		vIntervalValues.push_back( atoi( db.m_ppResults[i][0].c_str() ) );

	if( nTotalPieces == 0 || nIntervals == 0 )
		return;

	set<int> *psPieces = new set<int>[nIntervals];
	int *pnNumPieces = new int[nIntervals];
	for( int i = 0; i < nIntervals; i++ )
		pnNumPieces[i] = 0;

	vector<bool> vValidPieces;
	for( int i = 0; i < nTotalPieces; i++ )
		vValidPieces.push_back( true );
	int nNewPiece;

	// Set how many of the pieces will be unseeded
	if( nTotalPieces > 0 )
	{
		pnNumPieces[0] = (int)( randInt( 1, 80 ) * 0.001 * nTotalPieces );
		pnNumPieces[0] = max( 1, pnNumPieces[0] );
		nTotalPieces -= pnNumPieces[0];
	}

	// Set how many of the pieces will be seeded
	for( int i = 1; i < nIntervals; i++ )
	{
		if( nTotalPieces > 0 )
		{
			pnNumPieces[i] = (int)( randInt( 25, 35 ) * 0.01 * nTotalPieces );
			pnNumPieces[i] = max( 1, pnNumPieces[i] );
			nTotalPieces -= pnNumPieces[i];
		}
	}

	if( nTotalPieces > 0 )
	{
		pnNumPieces[nIntervals-1] += nTotalPieces;
		nTotalPieces = 0;
	}
	nTotalPieces = (int)m_vPieceHashes.size();

	for( int i = 0; i < nIntervals; i++ )
	{
		// Start assigning the actual pieces that seeded and unseeded
		while( (int)psPieces[i].size() != pnNumPieces[i] )
		{
			nNewPiece = randInt( 0, nTotalPieces - 1 );
			if( vValidPieces[ nNewPiece ] )
			{
				vValidPieces[ nNewPiece ] = false;
				psPieces[i].insert( nNewPiece );
			}
		}
	}
	
	for( int i = 0; i < nTotalPieces; i++ )
	{
		if( vValidPieces[ i ] )
		{
			TRACE( "void TorrentCreator::SetPieceValues(void): Missed piece %d, inserting it in s1stSeededPieces\n", i );
			psPieces[1].insert( i );
		}
	}

	// Set the seedless pieces
	string query = "INSERT INTO bt_generator_seedless_pieces (torrent_id, piece_num) VALUES ";
	char aValue[64];
	while( !psPieces[0].empty() )
	{
		if( psPieces[0].size() != 1 )
			sprintf( aValue, "(%u, %d),", torrent_id, *(psPieces[0].begin()) );
		else
			sprintf( aValue, "(%u, %d)", torrent_id, *(psPieces[0].begin()) );

		query += aValue;
		psPieces[0].erase( psPieces[0].begin() );
	}
	db.Query( query.c_str(), false );

	// Set the seeded pieces
	for( int i = 1; i < nIntervals; i++ )
	{
		query = "INSERT INTO bt_generator_seeded_pieces (torrent_id, piece_num, time_interval) VALUES ";
		while( !psPieces[i].empty() )
		{
			if( psPieces[i].size() != 1 )
				sprintf( aValue, "(%u, %d, %d),", torrent_id, *(psPieces[i].begin()), vIntervalValues[i] );
			else
				sprintf( aValue, "(%u, %d, %d)", torrent_id, *(psPieces[i].begin()), vIntervalValues[i] );

			query += aValue;
			psPieces[i].erase( psPieces[i].begin() );
		}
		db.Query( query.c_str(), false );
	}

	delete [] pnNumPieces;
	delete [] psPieces;
}

//
// Writes the info dictionary into the OutputData File. The OutputData file is
//  then read in to compute the info_hash
//
void TorrentCreator::WriteTorrentInfoFile(vector<DataFile *> vDatafiles)
{
	m_tempOutputFile.SetLength(0);
	m_tempOutputFile.SeekToBegin();

	BDictionary infoDict;
	string key;
	BDictionaryItem *filesItem = NULL;
	BList *b_files = NULL;
	vector<BDictionary *> v_b_file_dict;
	vector<BDictionaryItem *> v_lengthItem;
	vector<BDictionaryItem *> v_pathItem;
	vector<BList *> v_b_pathlist;

	// IF there is only one file in the torrent
	if( vDatafiles.size() == 1 )
	{
		DataFile *datafile = vDatafiles.front();

		//Add lengthItem
		BInteger *fileLen = datafile->getBFileLen();
		BDictionaryItem *lengthItem = new BDictionaryItem();
		string key = "length";
		lengthItem->addIntegerItem( fileLen, key );
		infoDict.addItem( lengthItem );
		v_lengthItem.push_back( lengthItem );
	}
	// ELSE there are multiple files in the torrent
	else
	{
		filesItem = new BDictionaryItem;
		b_files = new BList; //b_files is a Blist of the BDictionaries b_file_dict.

		// FOR each file in the torrent
		for( vector<DataFile *>::iterator v_iter = vDatafiles.begin(); v_iter != vDatafiles.end(); v_iter++ )
		{
			DataFile* datafile = *v_iter;
			BDictionary* b_file_dict = new BDictionary();

			BInteger * fileLen = datafile->getBFileLen();
			BDictionaryItem * lengthItem = new BDictionaryItem();
			string key = "length";
			lengthItem->addIntegerItem(fileLen, key);
			b_file_dict->addItem(lengthItem);
			v_lengthItem.push_back(lengthItem);

			BDictionaryItem * pathItem = new BDictionaryItem(); // path is a Blist of BStrings of
			//the directory hierarchy and the filename
			key = "path";
			BList * b_pathlist = new BList();
			vector <DataPath *> v_datapath = datafile->get_v_datapath();
			vector<DataPath *>::iterator v_iter2;
			for(v_iter2 = v_datapath.begin(); v_iter2 != v_datapath.end( ) ;v_iter2++) {
				BString *dir = (*v_iter2)->GetBDir();
				b_pathlist->addItem(dir);
			}
			//add filename to end of the pathlist
			BString *filename = datafile->getBFilename();
			b_pathlist->addItem(filename);
			pathItem->addListItem(b_pathlist, key);
			v_b_pathlist.push_back(b_pathlist);
			b_file_dict->addItem(pathItem);
			v_pathItem.push_back(pathItem);

			b_files->addItem(b_file_dict);
			v_b_file_dict.push_back(b_file_dict);
		}
		key = "files";
		filesItem->addListItem( b_files, key );
		infoDict.addItem(filesItem);	
	}

	BDictionaryItem *root_item = new BDictionaryItem();
	key = "name";
	BString *root = m_pIPData->GetBParentDataname();
	root_item->addStringItem(root, key);
	infoDict.addItem(root_item);

	BDictionaryItem pieceLen;
	BInteger *plen = m_pIPData->GetBPieceLength();
	key = "piece length";
	pieceLen.addIntegerItem(plen, key);
	infoDict.addItem(&pieceLen);

	// Am not adding the hashes to the infoDict for it is UCHAR data that I do not want to
	//  convert to a signed char or string data.
	infoDict.WritePartialEncodedData( &m_tempOutputFile );

	key = "pieces";
	BString keyStr(key);
	keyStr.WriteEncodedData( &m_tempOutputFile );
	int pieces_size = (int)(m_vPieceHashes.size()) * 20;

	char aBuf[ sizeof(int)*8 + 1 ];
	int nLen = sprintf( aBuf, "%d%c", pieces_size, ':' );
	m_tempOutputFile.Write( aBuf, nLen );

	UCHAR *pAllPieces = new UCHAR[ pieces_size ];
	int pos =0;
	for( vector<UCHAR *>::iterator v_iter = m_vPieceHashes.begin(); v_iter != m_vPieceHashes.end(); v_iter++ )
	{
		memcpy( &pAllPieces[pos], *v_iter, 20 );
		pos += 20;
	}
	m_tempOutputFile.Write( pAllPieces, pieces_size );
	delete pAllPieces;

	//end the info dictionary
	char end = 'e';
	m_tempOutputFile.Write( &end, 1 );

	delete root_item;
	for( vector <BList *>::iterator v_iter = v_b_pathlist.begin(); v_iter != v_b_pathlist.end(); v_iter++ )
	{
		delete (*v_iter);
	}
	v_b_pathlist.clear();

	vector <BDictionaryItem *>::iterator v_iter2;
	for( v_iter2 = v_pathItem.begin(); v_iter2 != v_pathItem.end(); v_iter2++) {
		delete (*v_iter2);
	}
	v_pathItem.clear();

	for( v_iter2 = v_lengthItem.begin(); v_iter2 != v_lengthItem.end(); v_iter2++) {
		delete (*v_iter2);
	}
	v_lengthItem.clear();

	vector <BDictionary *>::iterator v_iter4;
	for( v_iter4 = v_b_file_dict.begin(); v_iter4 != v_b_file_dict.end(); v_iter4++) {
		delete (*v_iter4);
	}
	v_b_file_dict.clear();

	if (b_files != NULL)
		delete b_files;
	if (filesItem != NULL)
		delete filesItem;
}

void TorrentCreator::WriteOutputToFile(void)
{
	string torrentFilename = "C:\\BTTorrentGenerator\\";
	torrentFilename += m_pIPData->GetTorrentFileName();

	CFile torrentFile;
	if( !torrentFile.Open( torrentFilename.c_str(), CFile::modeCreate | CFile::modeWrite ) )
	{
		TRACE( "Error opening output file to write torrent\n" );
		return;
	}
	torrentFile.SetLength(0);

	BDictionary torrentDict;

	vector<BString *> v_trackers = m_pIPData->GetTrackerURLs();
	BDictionaryItem *announce_item = new BDictionaryItem();
	string key = "announce";
	announce_item->addStringItem(v_trackers[0], key);
	torrentDict.addItem(announce_item);

	BDictionaryItem *pAnnounceListDictItem = NULL;
	vector<BList *> vpUsedLists;
	if( v_trackers.size() > 1 )
	{
		pAnnounceListDictItem = new BDictionaryItem();
		BList *pAnnounceList = new BList();
		vpUsedLists.push_back( pAnnounceList );
		key = "announce-list";

		BList *pAnnounceListItem = new BList();
		vpUsedLists.push_back( pAnnounceListItem );
		pAnnounceListItem->addItem( v_trackers[0] );
		pAnnounceList->addItem( pAnnounceListItem );

		pAnnounceListItem = new BList();
		vpUsedLists.push_back( pAnnounceListItem );
		pAnnounceListItem->addItem( v_trackers[1] );
		pAnnounceList->addItem( pAnnounceListItem );

		if( v_trackers.size() == 3 )
		{
			pAnnounceListItem = new BList();
			vpUsedLists.push_back( pAnnounceListItem );
			pAnnounceListItem->addItem( v_trackers[2] );
			pAnnounceList->addItem( pAnnounceListItem );
		}
		
		pAnnounceListDictItem->addListItem( pAnnounceList, key );
		torrentDict.addItem( pAnnounceListDictItem );
	}
	v_trackers.clear();

	BInteger *creation_date = m_pIPData->GetCreationTime();
	BDictionaryItem * creation_date_item = new BDictionaryItem();
	key = "creation date";
	creation_date_item->addIntegerItem(creation_date, key);
	torrentDict.addItem(creation_date_item);

	torrentDict.WritePartialEncodedData( &torrentFile );
	unsigned int pos = (unsigned int)torrentFile.GetPosition();

	char buffer[7] = { '4', ':', 'i', 'n', 'f', 'o', '\0' }; 
	torrentFile.Write( buffer, 6 );
	pos = (unsigned int)torrentFile.GetPosition();

	//Write the info dictionary from the buffer.
	torrentFile.Write( m_pInfoBuffer, m_nInfoBufferSize );

	//end the torrentDict
	char end = 'e';
	torrentFile.Write( &end, 1 );
	torrentFile.Close();

	delete announce_item;
	delete creation_date_item;
	if( pAnnounceListDictItem != NULL )
	{
		delete pAnnounceListDictItem;
		pAnnounceListDictItem = NULL;
	}
	for( size_t i = 0; i < vpUsedLists.size(); i++ )
		delete vpUsedLists[i];
	vpUsedLists.clear();
}

//
//
//
void TorrentCreator::WriteTorrentToDatabase(void)
{
	// Open file
	char aFilename[256];
	sprintf( aFilename, "C:\\BTTorrentGenerator\\%s", m_pIPData->GetTorrentFileName().c_str() );
	CFile torrentFile;
	if( !torrentFile.Open( aFilename, CFile::modeRead | CFile::shareDenyWrite ) )
	{
		TRACE( "Error opening output file to write torrent\n" );
		return;
	}

	// Read in data
	char *pUnescapedTorrentFile = new char[ (size_t)torrentFile.GetLength() ];
	torrentFile.Read( pUnescapedTorrentFile, (UINT)torrentFile.GetLength() );

	char *pTorrentQuery = new char[ (2 * (size_t)torrentFile.GetLength()) + 256 ];
	sprintf( pTorrentQuery, "INSERT INTO bt_generator_torrentfile values ( %u, %u, '",
		m_pIPData->GetTorrentID(), (unsigned int)torrentFile.GetLength() );

	size_t j = strlen( pTorrentQuery );
	for( size_t i = 0; i < (size_t)torrentFile.GetLength(); i++ )
	{
		// IF the character has to be escaped
		if( pUnescapedTorrentFile[i] == '\0' )
		{
			pTorrentQuery[j++] = '\\';
			pTorrentQuery[j++] = '0';
		}
		else if( pUnescapedTorrentFile[i] == '\\' || pUnescapedTorrentFile[i] == '\'' ||
			pUnescapedTorrentFile[i] == '\"' )
		{
			pTorrentQuery[j++] = '\\';
			pTorrentQuery[j++] = pUnescapedTorrentFile[i];
		}
		// ELSE no escape is needed
		else
		{
			pTorrentQuery[j++] = pUnescapedTorrentFile[i];
		}
	}
	pTorrentQuery[j++] = '\'';
	pTorrentQuery[j++] = ' ';
	pTorrentQuery[j++] = ')';

	TinySQL db;
	if( !db.Init( IP_BITTORRENT_DATABASE, "onsystems", "ebertsux37", "bittorrent_data", 3306 ) || !db.BinaryQuery( pTorrentQuery, j ) )
	{
		TRACE( "TorrentCreator::WriteTorrentToDatabase(void): Error writing torrent file to database\n" );
	}

	torrentFile.Close();

	delete pUnescapedTorrentFile;
	delete pTorrentQuery;
}

//
//
//
string TorrentCreator::GetHashString(UCHAR * hashVal)
{
	char retStr[61];
	retStr[0] = 0;
	char szTemp[4];
	char szTemp2[4];
	char c[2];
	c[1] = 0;

	USHORT m_info_hash[20];
	for(int i = 0; i < HASH_SIZE; i++)
	{
		sprintf(szTemp, "%u", hashVal[i]);
		int value = atoi(szTemp);
		m_info_hash[i] = value;
		if((value > 0 && value < 255) && (isalnum(value))) {
			c[0] = value;
			strcat(retStr, c);
		}
		else {
			c[0] = '%';
			strcat(retStr, c);
			if(value < 16) strcat(retStr, "0");
			itoa(value, szTemp, 16);
			strcat(retStr, szTemp);
		}
	}

	USHORT hash[40];
	memcpy(hash, m_info_hash, 20*sizeof(USHORT));
	char retVal[41];
	strcpy(szTemp2, "");
	strcpy(retVal, "");
	for(int i = 0; i < 20; i++) {
		int value = hash[i];
		if(value < 16) strcat(retVal, "0");
		itoa(value, szTemp2, 16);
		strcat(retVal, szTemp2);
	}
	m_hashString = retVal;
	TRACE( "m_hash_string2 is %s\n", m_hashString.c_str() );
	return string(retVal);

}

//
//
//
size_t TorrentCreator::GetTorrents(vector <BTInput *> &rvTorrents)
{
	// Clear the vector of torrents
	if( !rvTorrents.empty() )
	{
		for( vector <BTInput *>::iterator v_iter = rvTorrents.begin(); v_iter != rvTorrents.end(); v_iter++ )
			delete (*v_iter);
		rvTorrents.clear();
	}

	TinySQL db;
	db.Init( IP_BITTORRENT_DATABASE, "onsystems", "ebertsux37", "bittorrent_data", 3306 );

	CHAR aQuery[1024];
	sprintf( aQuery, "SELECT torrentfilename,auto_torrent_id,piece_length,total_length,topmost_file_or_dir,zero_seed_inflate FROM bt_generator_maininfo WHERE auto_torrent_id > %u AND active='T' AND generate='T' ORDER BY auto_torrent_id ASC", m_nLastAutoID );
	db.Query( aQuery, true );
	for( int i = 0; i < (int)db.m_nRows; i++ )
	{
		BTInput *pBTInput = new BTInput( db.m_ppResults[i][0], atol( db.m_ppResults[i][1].c_str() ) );

		pBTInput->SetPieceLength( atol( db.m_ppResults[i][2].c_str() ) );
		pBTInput->SetTotalSize( atol( db.m_ppResults[i][3].c_str() ) );

		pBTInput->SetParentDataname( db.m_ppResults[i][4].c_str() );

		pBTInput->SetZeroSeedInflate( db.m_ppResults[i][5].compare( "T" ) == 0 );

		rvTorrents.push_back( pBTInput );
		m_nLastAutoID = atol( db.m_ppResults[i][1].c_str() );
	}

	for( int i = 0; i < (int)rvTorrents.size(); i++ )
	{
		// Get announce URLs
		sprintf( aQuery, "SELECT primary_tracker_url, secondary_tracker_url, tertiary_tracker_url FROM bt_generator_announcelist WHERE torrent_id = %u", m_nLastAutoID );
		if( !db.Query( aQuery, true ) )
		{
			delete rvTorrents[i];
			rvTorrents[i] = NULL;
			continue;
		}

		if( db.m_nRows == 1 )
		{
			if( !db.m_ppResults[0][0].empty() )
				rvTorrents[i]->AddTrackerURL( new BString( db.m_ppResults[0][0] ) );
			if( !db.m_ppResults[0][1].empty() )
				rvTorrents[i]->AddTrackerURL( new BString( db.m_ppResults[0][1] ) );
			if( !db.m_ppResults[0][2].empty() )
				rvTorrents[i]->AddTrackerURL( new BString( db.m_ppResults[0][2] ) );
		}

		if( rvTorrents[i]->TrackerURLCount() == 0 )
		{
			rvTorrents[i]->AddTrackerURL( new BString( string( "http://127.0.0.1:6969/announce" ) ) );
		}

		sprintf( aQuery, "SELECT file_length, md5sum, file_name, path FROM bt_generator_datafile WHERE torrent_id = %u ORDER BY file_number ASC",
			rvTorrents[i]->GetTorrentID() );
		if( !db.Query( aQuery, true ) )
		{
			delete rvTorrents[i];
			rvTorrents[i] = NULL;
			continue;
		}

		for( int j = 0; j < (int)db.m_nRows; j++ )
		{
			rvTorrents[i]->AddDataFile( new DataFile( atol( db.m_ppResults[j][0].c_str() ), db.m_ppResults[j][1],
				db.m_ppResults[j][2], db.m_ppResults[j][3] ) );
		}
	}

	// Remove any NULL results
	for( int i = 0; i < (int)rvTorrents.size(); /* nothing */ )
	{
		if( rvTorrents[i] == NULL )
		{
			rvTorrents.erase( rvTorrents.begin() + i );
		}
		else
		{
			i++;
		}
	}

	return rvTorrents.size();
}

//
//
//
void TorrentCreator::WriteHashToDatabase(void)
{
	TRACE( "Start updating bt_generator_maininfo database\n" );

	CTime currentTime = CTime::GetCurrentTime();
	m_pIPData->SetCreationTime( (time_t)currentTime.GetTime() );

	DBInterface db;
	string torrentfilename = m_pIPData->GetTorrentFileName();
	db.OpenConnection(IP_BITTORRENT_DATABASE, "onsystems", "ebertsux37", "bittorrent_data");

	char aQuery[1024];
	sprintf( aQuery, "UPDATE bt_generator_maininfo SET info_hash = '%s', generate='F', creation_date = %s WHERE auto_torrent_id = %u",
		GetHashString(m_pIPData->GetInfohash()).c_str(), currentTime.Format( "%Y%m%d%H%M%S" ), m_pIPData->GetTorrentID() );
	int ret = db.executeQuery(aQuery);

	TRACE( "Done updating bt_generator_maininfo database\n" );
}

//
//
//
bool TorrentCreator::InitiateZeroSeedBoosting(void)
{
	if( m_pIPData->NeedsZeroSeedInflation() )
	{
		TinySQL db;
		if( db.Init( IP_BITTORRENT_DATABASE, "onsystems", "ebertsux37", "bittorrent_data", 3306 ) &&
			db.Query( "SELECT MAX(hash_id) FROM seed_inflation_data WHERE 200000 <= hash_id AND hash_id < 300000", true ) )
		{
			int nHashID = atoi( db.m_ppResults[0][0].c_str() );
			if( nHashID == 0 )
			{
				nHashID = 200000;
			}
			else
			{
				nHashID++;
			}
			
			char aQuery[512];
			sprintf( aQuery, "INSERT INTO seed_inflation_data VALUES (%d,'%s','%s','%s',0,'M','NULL')",
				nHashID, m_pIPData->GetTorrentFileName().c_str(), m_pIPData->GetPrimaryTrackerURL().c_str(),
				m_hashString.c_str() );

			return db.Query( aQuery, false );
		}
	}

	return false;
}
