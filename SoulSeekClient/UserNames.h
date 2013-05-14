#pragma once
#include <vector>
using namespace std;

class UserNames
{
public:
	UserNames(void);
	~UserNames(void);

	void PopulateUserList(void);
	vector<string>* GetRandomUserNames(void);
	string GetRandomUserName(void);
	vector<string>* GetUserNames(void);

	void PopulateDirectoryList(void);
	vector<string>* GetRandomDirectoryList(size_t size);
	vector<string>* GetDirectoryList();
private:
	vector<string> m_vUserList;
	vector<string> m_vDirectoryList;
	char fileLocation[MAX_PATH+1];
};
