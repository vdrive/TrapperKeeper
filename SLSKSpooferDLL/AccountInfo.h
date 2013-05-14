

#pragma once

class AccountInfo
{
public:
	char* un;
	char* pass;
	int port;
	int version;
	AccountInfo();
	~AccountInfo();
	void CreateAccount();
	void TerminateAccount();
};