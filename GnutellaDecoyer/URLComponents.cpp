// URLComponents.cpp

#include "stdafx.h"
#include "URLComponents.h"

//
//
//
URLComponents::URLComponents()
{
	// Initialize the buffers

	// Clear data
	memset(&m_data,0,sizeof(URL_COMPONENTS));
	m_data.dwStructSize=sizeof(URL_COMPONENTS);
		
	// Set buffer sizes
	m_data.dwExtraInfoLength=BUF_LEN;
	m_data.dwHostNameLength=BUF_LEN;
	m_data.dwPasswordLength=BUF_LEN;
	m_data.dwSchemeLength=BUF_LEN;
	m_data.dwUrlPathLength=BUF_LEN;
	m_data.dwUserNameLength=BUF_LEN;
		
	// Allocate buffers
	m_data.lpszExtraInfo=new char[BUF_LEN+1];
	m_data.lpszHostName=new char[BUF_LEN+1];
	m_data.lpszPassword=new char[BUF_LEN+1];
	m_data.lpszScheme=new char[BUF_LEN+1];
	m_data.lpszUrlPath=new char[BUF_LEN+1];
	m_data.lpszUserName=new char[BUF_LEN+1];
}

//
//
//
URLComponents::~URLComponents()
{
	// Free the buffers
	delete [] m_data.lpszExtraInfo;
	delete [] m_data.lpszHostName;
	delete [] m_data.lpszPassword;
	delete [] m_data.lpszScheme;
	delete [] m_data.lpszUrlPath;
	delete [] m_data.lpszUserName;
}
