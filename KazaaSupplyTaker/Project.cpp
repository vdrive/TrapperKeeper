// Project.cpp

#include "stdafx.h"
#include "Project.h"

//
//
//
Project::Project()
{
	Clear();
}

//
//
//
void Project::Clear()
{
	m_name.empty();
	m_name = "";
	m_query.empty();
	m_query = "";
	m_min_size=0;

	m_ip = "";
	m_login = "";
	m_pass = "";
	m_db = "";
	m_table = "";

	m_search_type=Everything;
	m_search_option=None;

	v_killwords.clear();

//	m_is_mpaa_project=false;
//	m_is_sony_project=false;
}