#pragma once
#include "stdafx.h"

class ProjectData
{
public:
	ProjectData(void) {
	}
	~ProjectData(void) {
	}
	string m_search_types; 
	string m_search_strings;
	string m_kill_words;
	string m_id; // project id's
	string m_owner;
	string m_min_size;
};
