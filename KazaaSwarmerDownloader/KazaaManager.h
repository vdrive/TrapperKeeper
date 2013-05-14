#pragma once

class SwarmerDownloader;

class KazaaManager
{
public:
	KazaaManager(void);
	~KazaaManager(void);

	void InitParent(SwarmerDownloader * parent);

	int CloseandRelaunchKazaa();
	void ClosePopups();
	void Click(HWND hwnd);
	int LaunchKazaa();
	void CheckforError();
	int KillKazaa();


private:
	
	SwarmerDownloader * p_parent;

};
