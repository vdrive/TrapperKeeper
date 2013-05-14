#ifndef BTSTRUCT
#define BTSTRUCT

#ifdef WIN32
	typedef __int64 int64;
	typedef unsigned __int64 uint64;
#else
	typedef long long int64;
	typedef unsigned long long uint64;
#endif

//#include "atom.h"
//#include "stdafx.h"
#include <string>
#include <vector>

using namespace std;

struct torrent_t
{
	string strInfoHash;
	string strName;
	string strLowerName;		// lower case version of strName
	int iComplete;
	int iDL;
	string strFileName;
	string strAdded;
	int64 iSize;
	int iFiles;
	int iComments;
	int64 iAverageLeft;
	int iAverageLeftPercent;
	int64 iMinLeft;
	int64 iMaxiLeft;
	string strTag;
	int iCompleted;
	int64 iTransferred;
	string strUploader;
	string strInfoLink;
};

struct peer_t
{
	string strIP;
	int64 iUpped;
	int64 iDowned;
	int64 iLeft;
	unsigned long iConnected;
	int iHop;
	float flShareRatio;
};

struct AtomDicti
{
	vector<string> v_Keys;
	vector<int >	v_Values;

};

struct AtomList
{
	vector<string> v_elements;
};

struct response_t
{
	string strCode;
	AtomDicti *pHeaders;
	string strContent;
};

struct tag_t
{
	string strTag;
	string strImage;
};


struct mime_t
{
	string strExt;
	string strMime;
};

struct user_t
{
	string strLogin;
	string strLowerLogin;
	string strMD5;
	string strMail;
	string strLowerMail;
	string strCreated;
	int iAccess;
};

#endif	