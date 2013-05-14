#include "StdAfx.h"
#include "BList.h"

#include "BString.h"
#include "BInteger.h"
#include "BDictionary.h"

BList::BList(void)
{
	m_btype=BEncoding::btype::list;
}

BList::~BList(void)
{
	//for( size_t i = 0; i < v_items.size(); i++ )
	//{
	//	delete v_items[i];
	//}
	v_items.clear();
}

void BList::addItem(BEncoding * item)
{
	v_items.push_back(item);
}

//
//
//
void BList::WriteEncodedData(CFile *pOutputFile)
{
	if( pOutputFile == NULL )
		return;

	char encode = 'l';
	pOutputFile->Write( &encode, 1 );

	for( size_t i = 0; i < v_items.size(); i++ )
	{
		v_items[i]->WriteEncodedData( pOutputFile );
	}

	encode = 'e';
	pOutputFile->Write( &encode, 1 );
}

//
//
//
/* 
void BList::ReadSelf(CFile *file)
{
	btype b=CheckNextByte(file);

	while(b != BEncoding::btype::end)
	{
		switch(b)
		{
			case btype::string:
			{
				BString *bstring=new BString();
				bstring->ReadSelf(file);
				v_items.push_back(bstring);
				break;
			}
			case btype::integer:
			{
				BInteger *bint=new BInteger();
				bint->ReadSelf(file);
				v_items.push_back(bint);
				break;
			}
			case btype::list:
			{
				BList *blist=new BList();
				blist->ReadSelf(file);
				v_items.push_back(blist);
				break;
			}
			case btype::dictionary:
			{
				BDictionary *bdictionary=new BDictionary();
				bdictionary->ReadSelf(file);
				v_items.push_back(bdictionary);
				break;
			}
			default:
			{
				// IT SHOULD NEVER GET HERE
				break;
			}
		}

		b=CheckNextByte(file);
	}	
}*/