#pragma once

#ifndef KAZAALAUNCHER
#define KAZAALAUNCHER

class KazaaLauncher
{
public:
	KazaaLauncher(void);
	~KazaaLauncher(void);
	void Launch(int _directoryIndex);

	CString m_kza_dir;
};

#endif