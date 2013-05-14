#pragma once

class ProcessDirectory
{
public:
	ProcessDirectory(void);
	~ProcessDirectory(void);
	DWORD m_process_id;
	string m_process_running_folder;
};
