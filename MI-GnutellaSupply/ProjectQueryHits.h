#pragma once
#include "QueryHit.h"

struct ProjectQueryHits
{
public:
	string m_project_name;
	vector<QueryHit> v_hits;
};