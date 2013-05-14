#pragma once
#include "..\tkcom\object.h"

class AresProjectInfo : public Object
{	
public:
	AresProjectInfo(const char* name, const char* owner, const char* artist,int id);
	~AresProjectInfo(void);

	string m_name;
	string m_owner;
	string m_artist;
	int m_id;
};
