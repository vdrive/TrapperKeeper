#include "StdAfx.h"
#include "ParentCacheIO.h"
#include "PacketIO.h"
#include "SoulSeekServerDll.h"

ParentCacheIO::ParentCacheIO()
{
	
}

ParentCacheIO::~ParentCacheIO(void)
{
	for(unsigned int i = 0; i < parents.size(); i++)
	{
		if(parents[i] != NULL) delete parents[i];
	}
}

//
//
//
void ParentCacheIO::InitParent(SoulSeekServerDll *parent)
{
	p_parent=parent;
}

void ParentCacheIO::Populate()
{
	strcpy( fileLocation, "ParentServers.txt" );
	maxServers = 1000;
	m_nextToken = 0;

	//Load Parents from text file
	CFile parentCache;
	CFileException fileException;

	char seps[]   = "\t\n";
	char *token;

	if ( !parentCache.Open( fileLocation, CFile::modeCreate |   
		CFile::modeNoTruncate | CFile::modeRead | CFile::shareDenyWrite, &fileException ) )
	{
		TRACE( "Can't open file %s, error = %u\n",
			fileLocation, fileException.m_cause );
		return;
	}

	ULONGLONG f_len = parentCache.GetLength();
	if( f_len == 0) return;

	char *data;
	data = new char[(size_t)f_len + 1];
    data[f_len] = '\0';
	parentCache.Read(data, (UINT)f_len);
	token = strtok( data, seps );

	while( token != NULL )
	{
		char* getUsername = token;
		char* getIP = strtok( NULL, seps );
		char* getPort = strtok( NULL, seps );
		int intPort;
		intPort = atoi(getPort);
		char* getTries = strtok( NULL, seps );
		int intTries;
		intTries = atoi(getTries);
		parents.push_back( new Client(getUsername, getIP, intPort, intTries, 0, m_nextToken) );
		m_nextToken++;
		/* Get next token: */
		token = strtok( NULL, seps );
	}

	p_parent->m_dlg.SetParentSocketData(0,m_nextToken,0,0,0);

	parentCache.Close();
	delete[] data;
}

unsigned int ParentCacheIO::AddParent(char* username, char* ip, int port, int socketStatus)
{

	CFile parentCache;
	CFileException fileException;

	if ( !parentCache.Open( fileLocation, CFile::modeCreate |   
			CFile::modeWrite | CFile::shareDenyWrite, &fileException ) )
	{
		TRACE( "Can't open file %s, error = %u\n",
			fileLocation, fileException.m_cause );
		return 0;
	}

	for(unsigned int i = 0; i < parents.size(); i++)
	{
		char lineToWrite[512];

		Client* t_parent = parents.at(i);
		//check to see if that parent contains similar information
		if(t_parent->Compare(username, ip, port))
		{
			//Remove parent if the new parent is similar to it
			delete parents[i];
			parents.erase(parents.begin() + i);
		}
		else
		{
			//If the added entry is different then write the parent
			parentCache.SeekToEnd();
			sprintf(lineToWrite, "%s\t%s\t%i\t%i\r\n", t_parent->GetUsername(), t_parent->GetIP(), t_parent->GetPort(), t_parent->GetTries());
			parentCache.Write(lineToWrite, (UINT)strlen( lineToWrite ));
			//TRACE(lineToWrite);
		}
	}

	//Limit the number of servers to connect to
	//Remove the beginning since its the oldest
	if(parents.size() == maxServers && parents.size() != 0) 
	{
		parents.erase(parents.begin());
	}
	else
	{
		p_parent->m_dlg.SetParentSocketData(0,1,0,0,0);
	}
	//Write Added entry
	parentCache.SeekToEnd();
	char temp[512];
	sprintf(temp, "%s\t%s\t%i\t0\r\n", username, ip, port);
	parentCache.Write(temp, (UINT)strlen( temp ));
	parents.push_back( new Client(username, ip, port, 1, socketStatus, m_nextToken) );

	parentCache.Close();	

	m_nextToken++;
	return (m_nextToken - 1);
}

//
//
//
void ParentCacheIO::ReceivedConnection(unsigned int token)
{
	for(unsigned int i = 0; i < parents.size(); i++)
	{
		if(parents[i]->GetToken() == token)
		{
			parents[i]->ResetToken();
			parents[i]->SetStatus(2); //we are connected
			parents[i]->ResetTries();
			WriteFile();
			return;
		}
	}
	TRACE("Unknow Token");
        
}

//
//
//
void ParentCacheIO::FailedConnection(unsigned int token)
{
	for(unsigned int i = 0; i < parents.size(); i++)
	{
		if(parents[i]->GetToken() == token)
		{
			parents[i]->ResetToken();
			parents[i]->SetStatus(0); //we are not connected
			parents[i]->IncrementTries();
			if(parents[i]->GetTries() >= 5)
			{
				p_parent->m_dlg.SetParentSocketData(0,-1,0,0,0);
				delete parents[i];
				parents.erase(parents.begin() + i);
			}
			WriteFile();
			return;
		}
	}
	TRACE("Unknow Token");
        
}
int ParentCacheIO::RemoveFailedConnections()
{
	//CTime time;
	time_t currentTime;// = (time_t)time.GetTime();
	time(&currentTime);
	int counter = 0;
	for(unsigned int i = 0; i < parents.size(); i++)
	{
		if(currentTime - parents[i]->GetTime() >= 60*5)
		{
			parents[i]->ResetToken();
			parents[i]->SetStatus(0); //we are not connected
			parents[i]->IncrementTries();
			if(parents[i]->GetTries() >= 5)
			{
				p_parent->m_dlg.SetParentSocketData(0,-1,0,0,0);
				delete parents[i];
				parents.erase(parents.begin() + i);
				i--;
			}
			counter++;
		}
	}
	WriteFile();
	return counter;
}
int ParentCacheIO::RemoveBadServers()
{
	int counter = 0;
	for(unsigned int i = 0; i < parents.size(); i++)
	{
		if(parents[i]->GetTries() >= 5)
		{
			delete parents[i];
			parents.erase(parents.begin() + i);
			i--;
			counter++;
		}
	}
	WriteFile();
	return counter;
}
unsigned int ParentCacheIO::GetNextToken()
{
	return m_nextToken;
}

void ParentCacheIO::WriteFile()
{
	CFile parentCache;
	CFileException fileException;

	if ( !parentCache.Open( fileLocation, CFile::modeCreate |   
			CFile::modeWrite | CFile::shareDenyWrite, &fileException ) )
	{
		TRACE( "Can't open file %s, error = %u\n",
		fileLocation, fileException.m_cause );
		return;
	}

	for(unsigned int i = 0; i < parents.size(); i++)
	{
		char lineToWrite[512];

		Client* t_parent = parents.at(i);
		//If the added entry is different then write the parent
		parentCache.SeekToEnd();
		sprintf(lineToWrite, "%s\t%s\t%i\t%i\r\n", t_parent->GetUsername(), t_parent->GetIP(), t_parent->GetPort(), t_parent->GetTries());
		parentCache.Write(lineToWrite, (UINT)strlen( lineToWrite ));
	}
}

void ParentCacheIO::ConnectToParents()
{
	for(unsigned int i = 0; i < parents.size(); i++)
	{
		if(parents[i]->GetStatus() == 0)
		{
			char* username = parents[i]->GetUsername(); 

			PacketWriter packet((int)(17+strlen(username)));
			packet.AppendInt(18,false); // CMD code
			packet.AppendInt(parents[i]->GetToken(),false);  // Token
			packet.AppendSizeAndString(username,false); // username
			packet.AppendInt(1,false);
			packet.AppendChar('D',false);
			
			p_parent->m_socket.SendSocketData( packet.GetPacket(), packet.GetPacketSize() );
			char temp[512];
			sprintf(temp, "Requesting PARENT CONNECT to us: %s", parents[i]->GetIP());
			p_parent->m_dlg.WriteToLog(0,MAIN_SERVER_NUM,temp);
		}
	}
}