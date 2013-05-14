#include "StdAfx.h"
#include "DataPath.h"
#include "BString.h"

//
//
//
DataPath::DataPath(const string &rDir)
: m_dir(rDir)
{
	m_pBDir = new BString( rDir );
}

//
//
//
DataPath::~DataPath(void)
{
	if( m_pBDir != NULL )
	{
		delete m_pBDir;
		m_pBDir = NULL;
	}
}

//
//
//
string DataPath::GetDir(void)
{
	return m_dir;
}

//
//
//
BString *DataPath::GetBDir(void)
{
	return m_pBDir;
}
