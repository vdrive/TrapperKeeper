#pragma once

class Client
{
public:
	Client(char* theUsername, char* theIP, int thePort, int numOfTries, int socketStatus, unsigned int theToken);
	~Client(void);
	void SetInfo(char* theUsername, char* theIP, int thePort, int numOfTries, int socketStatus, unsigned int theToken);
	bool Compare(char* theUsername, char* theIP, int thePort);
	char* GetUsername(void);
	char* GetIP(void);
	int GetPort(void);

	int GetStatus(void);
	void SetStatus(int newStatus);

	unsigned int GetToken(void);
	void SetToken(int newToken);
	void ResetToken(void);

	int GetTries(void);
	void ResetTries(void);
	void IncrementTries(void);

	time_t GetTime(void);

private:
	char* username;
	char* ip;
	int port;
	int status; // 0 NotConnected, 1 Connecting, 2 Connected 
	unsigned int token;  // 0 if reset
	int tries;  // 0 If not tried to connect
	time_t lastAttempt;
};
