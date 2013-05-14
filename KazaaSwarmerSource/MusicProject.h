#pragma once
#include "fileentry.h"


class MusicProject
{
public:
	MusicProject(void);
	~MusicProject(void);

	char m_projectname[256]; 
	int m_track;
	int m_minswarms;
	int m_maxswarms;
	int m_found;
};
