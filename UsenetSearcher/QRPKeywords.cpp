// QRPKeywords.cpp

#include "stdafx.h"
#include "QRPKeywords.h"

//
//
//
QRPKeywords::QRPKeywords()
{
	Clear();
}

//
//
//
QRPKeywords::~QRPKeywords()
{
}

//
//
//
void QRPKeywords::Clear()
{

	v_keywords.clear();

}

//
//
//
int QRPKeywords::GetBufferLength()
{
/*
	// Public Data Members
	vector<string> v_keywords;
	vector<string> v_exact_keywords;
	vector<string> v_killwords;
	vector<string> v_exact_killwords;
*/
	int i;
	int len=0;

	len+=sizeof(unsigned int);	// number of keywords
	for(i=0;i<(int)v_keywords.size();i++)
	{
		len+=(int)v_keywords[i].size()+1;	// +1 for the NULL
	}

	return len;
}

//
//
//
int QRPKeywords::WriteToBuffer(char *buf)
{

	int i;

	// Clear the buffer
	memset(buf,0,GetBufferLength());
	
	char *ptr=buf;

	*((int *)ptr)=(int)v_keywords.size();
	ptr+=sizeof(int);
	for(i=0;i<(int)v_keywords.size();i++)
	{
		strcpy(ptr,v_keywords[i].c_str());
		ptr+=v_keywords[i].size()+1;	// +1 for NULL
	}

	return GetBufferLength();
}

//
//
//
int QRPKeywords::ReadFromBuffer(char *buf)
{

	int i;
	int len;
	
	Clear();

	char *ptr=buf;

	len=*((int *)ptr);
	ptr+=sizeof(int);
	for(i=0;i<len;i++)
	{
		string keyword;
		keyword = ptr;
		v_keywords.push_back(keyword);
		ptr+=strlen(ptr)+1;	// +1 for NULL
	}

	return GetBufferLength();
}