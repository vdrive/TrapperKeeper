#include "StdAfx.h"
#include "client.h"

Client::Client(char* theUsername, char* theIP, int thePort, int numOfTries, int socketStatus, unsigned int theToken)
{
	username = NULL;
	ip = NULL;
	SetInfo(theUsername,theIP,thePort,numOfTries, socketStatus, theToken);
}

Client::~Client(void)
{
	if(username != NULL) 
	{
		delete[] username;
		username = NULL;
	}
	if(ip != NULL)
	{
		delete[] ip;
		ip = NULL;
	}
}

void Client::SetInfo(char* theUsername, char* theIP, int thePort, int numOfTries, int socketStatus, unsigned int theToken)
{
	username = new char[strlen(theUsername)+1];
	strcpy(username, theUsername);
	ip = new char[strlen(theIP)+1];
	strcpy(ip, theIP);
	port = thePort;
	tries = numOfTries;
	SetStatus(socketStatus);
	token = theToken;
}

bool Client::Compare(char* theUsername, char* theIP, int thePort)
{
	if(theUsername == username || (theIP==ip && thePort == port))
	{
		return true;
	}
	return false;
}

char* Client::GetUsername()
{
	return username;
}
char* Client::GetIP()
{
	return ip;
}
int Client::GetPort()
{
	return port;
}

int Client::GetStatus()
{
	return status;
}
void Client::SetStatus(int newStatus)
{
	status = newStatus;
	if(status == 1) 
	{
		//CTime time;
		time(&lastAttempt);// = (time_t)time.GetTime();
	}
	else 
	{
		lastAttempt = 0;
	}
}

unsigned int Client::GetToken()
{
	return token;
}
void Client::SetToken(int newToken)
{
	token = newToken;
}
void Client::ResetToken()
{
	token= -1;
}

int Client::GetTries()
{
	return tries;
}
void Client::ResetTries()
{
	tries = 0;
}
void Client::IncrementTries()
{
	tries++;
}

time_t Client::GetTime()
{
	return lastAttempt;
}