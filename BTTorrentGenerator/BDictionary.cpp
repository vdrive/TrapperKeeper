#include "StdAfx.h"
#include "BDictionary.h"

BDictionary::BDictionary(void)
{
	m_btype = BEncoding::btype::dictionary;
}

BDictionary::~BDictionary(void)
{
	//for( size_t i = 0; i < v_items.size(); i++ )
	//{
	//	delete v_items[i];
	//}
	v_items.clear();
}

void BDictionary::addItem(BDictionaryItem * item) {
	v_items.push_back(item);
}

void BDictionary::WriteEncodedData(CFile *pOutputFile)
{
	if( pOutputFile == NULL )
		return;

	char encode = 'd';
	pOutputFile->Write( &encode, 1 );

	for( vector <BDictionaryItem *>::iterator iter = v_items.begin(); iter != v_items.end(); iter++ )
		(*iter)->WriteEncodedData( pOutputFile );

	encode = 'e';
	pOutputFile->Write( &encode, 1 );
}

void BDictionary::WritePartialEncodedData(CFile *pOutputFile)
{
	if( pOutputFile == NULL )
		return;

	char encode = 'd';
	pOutputFile->Write( &encode, 1 );

	for( vector <BDictionaryItem*>::iterator iter = v_items.begin(); iter != v_items.end(); iter++ )
	{
		(*iter)->WriteEncodedData( pOutputFile );
	}
}

//
//
//
/* void BDictionary::ReadSelf(CFile *file)
{
	while(1)
	{
		// Check for end of dictionary
		char c;
		file->Read(&c,1);
		if(c=='e')
		{
			break;
		}
		file->Seek(-1,CFile::current);

		// It is not the end of the dictionary, so read in the next item
        BDictionaryItem *item=new BDictionaryItem();
		item->ReadSelf(file);
		v_items.push_back(item);
	}
}
*/