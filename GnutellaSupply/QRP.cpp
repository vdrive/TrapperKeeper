// QRP.cpp

//
// These functions were taken from the QRP document at http://www.limewire.com/developer/query_routing/keyword%20routing.htm
//

#include "stdafx.h"
#include "QRP.h"

//
//
//
QRP::QRP()
{
}

//
//
//
QRP::~QRP()
{
}

//
//
//
int QRP::Hash(char *buf,unsigned char bits)
{
	return Hash(buf,0,(int)strlen(buf),bits);
}

//
//
//
int QRP::Hash(char *buf,int start,int end,unsigned char bits)
{
	char *lbuf=new char[strlen(buf)+1];
	strcpy(lbuf,buf);
	strlwr(lbuf);

	int xor=0;
	int j=0;
	for(int i=start;i<end;i++)
	{
		int b=lbuf[i];
		b=b<<(j*8);
		xor^=b;
		j=(j+1)%4;
	}

	delete [] lbuf;

	return HashFast(xor,bits);
}

//
//
//
int QRP::HashFast(int x,byte bits)
{
	__int64 prod=(__int64)x*(__int64)0x4F1BBCDC;
	__int64 ret=prod<<32;
	
	ret=ret>>1;
	ret&=0x7FFFFFFFFFFFFFFF;
	ret=ret >> ((32+(32-bits))-1);
	
	return (int)ret;
}

//
//
//
void QRP::AddKeywordToTable(char *keyword,char *table)
{
	unsigned int hash=QRP::Hash(keyword,16);	// 16 bits - 64K table made up of 4 bits items

	// Morpheus 1.9.1.0 style
	table[hash]=-1;	// 0xFF
/*
	if(hash % 2)
	{
		table[hash/2]|=0x01;
	}
	else
	{
		table[hash/2]|=0x10;
	}
*/
}

//
//
//
void QRP::AddUpTo3KeywordsToTable(char *keyword,char *table)
{
	char *buf=new char[strlen(keyword)+1];
	strcpy(buf,keyword);
	
	for(int i=0;i<3;i++)
	{
		if(strlen(buf)>=1)
		{
			AddKeywordToTable(buf,table);
			buf[strlen(buf)-1]='\0';	// drop off the last letter
		}
		else
		{
			break;
		}
	}

	delete [] buf;
}