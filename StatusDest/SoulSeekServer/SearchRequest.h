#pragma once
#include <vector>
using namespace std;

class SearchRequest
{
public:
	SearchRequest(char* username, char* searchText, int token);
	~SearchRequest(void);

	char* GetUsername(void);
	char* GetSeachText(void);
	int GetToken(void);

	char* GetIP(void);
	int GetPort(void);

	void SetConnection(char* ip, int port);
	void SetFileName(vector<string>* fileName);
	vector<string>* GetFileName(void);

private:
	char* p_Username;
	char* p_UserIP;
	int m_UserPort;

	char* p_SearchText;
	int m_Token;

	vector<string>* p_FileName;
};
