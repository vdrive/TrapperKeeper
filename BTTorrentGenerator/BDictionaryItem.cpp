#include "StdAfx.h"
#include ".\bdictionaryitem.h"

#include "BInteger.h"
#include "BList.h"
#include "BDictionary.h"

BDictionaryItem::BDictionaryItem(void)
{
	p_bstring=NULL;
	p_binteger=NULL;
	p_blist=NULL;
	p_bdictionary=NULL;
}

BDictionaryItem::~BDictionaryItem(void)
{
	/* if(p_bstring!=NULL)
	{
		delete p_bstring;
		p_bstring = NULL;
	}
	if(p_binteger!=NULL)
	{
		delete p_binteger;
		p_binteger = NULL;
	}
	if(p_blist!=NULL)
	{
		delete p_blist;
		p_blist = NULL;
	}
	if(p_bdictionary!=NULL)
	{
		delete p_bdictionary;
		p_bdictionary= NULL;
	}
	*/
	
}

void BDictionaryItem::addStringItem(BString * item, std::string key) {
	if (p_bstring != NULL) {
		delete p_bstring;
	}
	p_bstring = item;
	m_key.SetString(key);
	
}

void BDictionaryItem::addIntegerItem(BInteger *item, std::string key) {
	if (p_binteger != NULL) {
		delete p_binteger;
	}
    p_binteger = item;
	m_key.SetString(key);
}

void BDictionaryItem::addListItem(BList *item, std::string key) {
	if (p_blist != NULL) {
		delete p_blist;
	}
	p_blist = item;
	m_key.SetString(key);
}

void BDictionaryItem::addDictionaryItem(BDictionary *item, std::string key) {
	if (p_bdictionary != NULL) {
		delete p_bdictionary;
	}
	p_bdictionary = item;
	m_key.SetString(key);
}

void BDictionaryItem::WriteEncodedData(CFile *pOutputFile)
{
	if( pOutputFile == NULL )
		return;

	m_key.WriteEncodedData( pOutputFile );

	if(p_bstring!=NULL)
	{
		p_bstring->WriteEncodedData( pOutputFile );
	}
	if(p_binteger!=NULL)
	{
		p_binteger->WriteEncodedData( pOutputFile );
	}
	if(p_blist!=NULL)
	{
		p_blist->WriteEncodedData( pOutputFile );
	}
	if(p_bdictionary!=NULL)
	{
		p_bdictionary->WriteEncodedData( pOutputFile );
	}
}

/*
//
//
//
BDictionaryItem::BDictionaryItem(const BDictionaryItem &item)
{
	p_bstring=NULL;
	p_binteger=NULL;
	p_blist=NULL;
	p_bdictionary=NULL;

	if(item.p_bstring!=NULL)
	{
		p_bstring=new BString(*item.p_bstring);
	}
	if(item.p_binteger!=NULL)
	{
		p_binteger=new BInteger();
		p_binteger->m_value=item.p_binteger->m_value;
	}
	if(item.p_blist!=NULL)
	{
		p_blist=new BList();
		for(unsigned int i=0;i<item.p_blist->v_items.size();i++)
		{
			p_blist->v_items.push_back(item.p_blist->v_items[i]);
		}
	}
	if(item.p_bdictionary!=NULL)
	{
		p_bdictionary=new BDictionary();
		for(unsigned int i=0;i<item.p_bdictionary->v_items.size();i++)
		{
			p_bdictionary->v_items.push_back(item.p_bdictionary->v_items[i]);
		}
	}

	m_type=item.m_type;

	m_key=item.m_key;
}

BDictionaryItem &BDictionaryItem::operator=(const BDictionaryItem &item)
{
	if(p_bstring!=NULL)
	{
		delete p_bstring;
		p_bstring=NULL;
	}
	if(p_binteger!=NULL)
	{
		delete p_binteger;
		p_binteger=NULL;
	}
	if(p_blist!=NULL)
	{
		delete p_blist;
		p_blist=NULL;
	}
	if(p_bdictionary!=NULL)
	{
		delete p_bdictionary;
		p_bdictionary=NULL;
	}

	if(item.p_bstring!=NULL)
	{
		p_bstring=new BString(*item.p_bstring);
	}
	if(item.p_binteger!=NULL)
	{
		p_binteger=new BInteger();
		p_binteger->m_value=item.p_binteger->m_value;
	}
	if(item.p_blist!=NULL)
	{
		p_blist=new BList();
		for(unsigned int i=0;i<item.p_blist->v_items.size();i++)
		{
			p_blist->v_items.push_back(item.p_blist->v_items[i]);
		}
	}
	if(item.p_bdictionary!=NULL)
	{
		p_bdictionary=new BDictionary();
		for(unsigned int i=0;i<item.p_bdictionary->v_items.size();i++)
		{
			p_bdictionary->v_items.push_back(item.p_bdictionary->v_items[i]);
		}
	}

	m_type=item.m_type;

	m_key=item.m_key;

	return *this;
}
*/
//
//
//
/* void BDictionaryItem::ReadSelf(CFile *file)
{
	if(p_bstring!=NULL)
	{
		delete p_bstring;
	}
	if(p_binteger!=NULL)
	{
		delete p_binteger;
	}
	if(p_blist!=NULL)
	{
		delete p_blist;
	}
	if(p_bdictionary!=NULL)
	{
		delete p_bdictionary;
	}

	// Read key string
	m_key.ReadSelf(file);

	// Read element
	m_btype=CheckNextByte(file);
	switch(m_btype)
	{
		case btype::string:
		{
			p_bstring=new BString();
			p_bstring->ReadSelf(file);
			break;
		}
		case btype::integer:
		{
			p_binteger=new BInteger();
			p_binteger->ReadSelf(file);
			break;
		}
		case btype::list:
		{
			p_blist=new BList();
			p_blist->ReadSelf(file);
			break;
		}
		case btype::dictionary:
		{
			p_bdictionary=new BDictionary();
			p_bdictionary->ReadSelf(file);
			break;
		}
		default:
		{
			// IT SHOULD NEVER GET HERE
			break;
		}
	}
}
*/

BString *BDictionaryItem::GetBString()
{
	return p_bstring;
}
BInteger *BDictionaryItem::GetBInteger()
{
	return p_binteger;
}
BList *BDictionaryItem::GetBList()
{
	return p_blist;
}
BDictionary *BDictionaryItem::GetBDictionary()
{
	return p_bdictionary;
}
string BDictionaryItem::GetKeyString()
{
	char *str = new char[m_key.Len()+1];
	memcpy(str, m_key.Buf(), m_key.Len());
	str[m_key.Len()] = '\0';

	std::string return_string = str;
	delete [] str;
	return return_string;
}