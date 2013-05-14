#include "stdafx.h"
#include "usernames.h"

UserNames::UserNames(void)
{
	m_vUserList.clear();
	m_vDirectoryList.clear();
	PopulateUserList();
	PopulateDirectoryList();
}

UserNames::~UserNames(void)
{
}

void UserNames::PopulateUserList()
{
	strcpy( fileLocation, "C:\\syncher\\rcv\\plug-ins\\soulseek\\namelist.txt" );

	//Load fake usernames from text file
	CFile userNameCache;
	CFileException fileException;

	char seps[] = "\r\n";
	char *token;

	if ( !userNameCache.Open( fileLocation, CFile::modeCreate |   
		CFile::modeNoTruncate | CFile::modeRead | CFile::shareDenyWrite, &fileException ) )
	{
		TRACE( "Can't open file %s, error = %u\n",
			fileLocation, fileException.m_cause );
		return;
	}

	ULONGLONG f_len = userNameCache.GetLength();
	if( f_len == 0) return;

	char *data;
	data = new char[(size_t)f_len + 1];
    data[f_len] = '\0';
	userNameCache.Read(data, (UINT)f_len);
	token = strtok( data, seps );

	while( token != NULL )
	{
		char temp_name[256];
		sprintf(temp_name,"%s%i",token,(int)strlen(token) % 10);
		//int index = temp_name.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_");
		//string new_name;
		//char temp_buf[2];
		//itoa(append, temp_buf, 10);
		//un += temp_buf;
		//int append = ( (int)temp_name.size() ) % 10;
		//itoa(append, temp_buf, 10);
		//temp_buf[1] = '\0';
        //new_name = temp_name;
		//new_name.append(temp_buf);
		//m_vUserList.push_back(string(temp_name));
		m_vUserList.push_back(string(temp_name));
		/* Get next token: */
		token = strtok( NULL, seps );
	}
	userNameCache.Close();
	delete[] data;
}

void UserNames::PopulateDirectoryList()
{
	strcpy( fileLocation, "C:\\syncher\\rcv\\plug-ins\\soulseek\\directorylist.txt" );

	//Load fake usernames from text file
	CFile directoryCache;
	CFileException fileException;

	char seps[] = "\r\n";
	char *token;

	if ( !directoryCache.Open( fileLocation, CFile::modeCreate |   
		CFile::modeNoTruncate | CFile::modeRead | CFile::shareDenyWrite, &fileException ) )
	{
		TRACE( "Can't open file %s, error = %u\n",
			fileLocation, fileException.m_cause );
		return;
	}

	ULONGLONG f_len = directoryCache.GetLength();
	if( f_len == 0) return;

	char *data;
	data = new char[(size_t)f_len + 1];
    data[f_len] = '\0';
	directoryCache.Read(data, (UINT)f_len);
	token = strtok( data, seps );

	while( token != NULL )
	{
		m_vDirectoryList.push_back(string(token));
		/* Get next token: */
		token = strtok( NULL, seps );
	}
	directoryCache.Close();
	delete[] data;
}
string UserNames::GetRandomUserName(void)
{
	return m_vUserList[rand()%((int)m_vUserList.size())];
}
vector<string>* UserNames::GetRandomUserNames()
{
	vector<string>* returnVector = new vector<string>;
	int numberOfUsers = rand()%4+5; //number between 5 and 8
	for(int i = 0; i < numberOfUsers;i++)
	{
		returnVector->push_back(m_vUserList[rand()%((int)m_vUserList.size())]);
	}
	return returnVector; 
}
vector<string>* UserNames::GetUserNames()
{
	return &m_vUserList; 
}

vector<string>* UserNames::GetRandomDirectoryList(size_t size)
{
	vector<string>* returnVector = new vector<string>;
	for(size_t i = 0; i < size;i++)
	{
		returnVector->push_back((string)m_vDirectoryList[rand()%((int)m_vDirectoryList.size())]);
	}
	return returnVector; 
}
vector<string>* UserNames::GetDirectoryList()
{
	return &m_vDirectoryList; 
}