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
int QRP::Hash(const char *buf,unsigned char bits)
{
	return Hash(buf,0,(int)strlen(buf),bits);
}

//
//
//
int QRP::Hash(const char *buf,int start,int end,unsigned char bits)
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
void QRP::AddKeywordToTable(const char *keyword,byte *table)
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
void QRP::AddUpTo3KeywordsToTable(const char *keyword,byte *table)
{
	if(strchr(keyword,'-')!=NULL)
	{
		vector<string> keywords;
		ExtractKeywordsFromQuery(&keywords,keyword);
		//CString tmp=keyword;
		//tmp.Remove('-');
		//keywords.push_back((string)tmp);
		for(UINT i=0;i<keywords.size();i++)
		{
			if(keywords[i].length()>=1)
				AddUpTo3KeywordsToTable(keywords[i].c_str(),table);
		}
	}
	char* buf=new char[strlen(keyword)+1];
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

void QRP::ExtractKeywordsFromQuery(vector<string> *keywords, const char *query)
{
	UINT i;

	// Make the string lowercase
	char *lbuf=new char[strlen(query)+1];
	strcpy(lbuf,query);
	//strlwr(lbuf);

	// Remove all non alpha-numeric characters
	ClearNonAlphaNumeric(lbuf,(int)strlen(lbuf));

	// Trim leading and trailing whitespace
	CString cstring=lbuf;
	cstring.TrimLeft();
	cstring.TrimRight();
	cstring.MakeLower();
	//string cstring_string=cstring;
	strcpy(lbuf,cstring);

	// Extract the keywords
	char *ptr=lbuf;
	while(strlen(ptr)>0)
	{
		// Skip past any intermediate spaces in between keywords
		while((*ptr==' '))
		{
			ptr++;
		}

		bool done=true;
		if(strchr(ptr,' ')!=NULL)	// see if there are more keywords after this keyword
		{
			done=false;
			*strchr(ptr,' ')='\0';
		}

		// Check to see if this keyword is already in there
		bool found=false;
		for(i=0;i<keywords->size();i++)
		{
			if(strcmp((*keywords)[i].c_str(),ptr)==0)
			{
				found=true;
				break;
			}
		}
		if(!found)	// if not, then add it
		{
			keywords->push_back(ptr);
		}

		if(done)
		{
			break;
		}
		else
		{
			ptr+=strlen(ptr)+1;
		}	
	}
	
	delete [] lbuf;
}

//
// Replaces all characters that are not letters or numbers with spaces in place
//
void QRP::ClearNonAlphaNumeric(char *buf,int buf_len)
{
	int i;
	for(i=0;i<buf_len;i++)
	{
		if(((buf[i]>='0') && (buf[i]<='9')) || ((buf[i]>='A') && (buf[i]<='Z')) || ((buf[i]>='a') && (buf[i]<='z')) || (buf[i]=='\''))
		{
			// It passes
		}
		else
		{
			buf[i]=' ';
		}
	}
}