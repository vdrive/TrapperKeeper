#include "StdAfx.h"
#include "searchrequest.h"

SearchRequest::SearchRequest(char* username, char* searchText,int token)
{
	p_Username = username;
	p_SearchText = searchText;
	m_Token = token;

	p_FileName = NULL;
	p_UserIP = NULL;
}

SearchRequest::~SearchRequest(void)
{
	if(p_Username != NULL)
	{
		delete p_Username;
		p_Username = NULL;
	}
	if(p_UserIP != NULL)
	{
		delete p_UserIP;
		p_UserIP = NULL;
	}
	if(p_SearchText != NULL)
	{
		delete p_SearchText;
		p_SearchText = NULL;
	}
	if(p_FileName != NULL)
	{
		delete p_FileName;
		p_FileName = NULL;
	}
}

char* SearchRequest::GetUsername()
{
	return p_Username;
}

char* SearchRequest::GetSeachText()
{
	return p_SearchText;
}


int SearchRequest::GetToken()
{
	return m_Token;
}

char* SearchRequest::GetIP(void)
{
	return p_UserIP;
}

int SearchRequest::GetPort()
{
	return m_UserPort;
}

void SearchRequest::SetConnection(char* ip, int port)
{
	if(p_UserIP != NULL)
		delete p_UserIP;
	p_UserIP = new char[16];
	strcpy(p_UserIP, ip);
	m_UserPort = port;
}

void SearchRequest::SetFileName(vector<string>* fileName)
{
	p_FileName = fileName;
}

vector<string>* SearchRequest::GetFileName()
{
	return p_FileName;
}