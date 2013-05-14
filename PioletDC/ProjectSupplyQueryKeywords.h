// ProjectSupplyQueryKeywords.h

#ifndef PROJECT_SUPPLY_QUERY_KEYWORDS_H
#define PROJECT_SUPPLY_QUERY_KEYWORDS_H

class ProjectSupplyQueryKeywords
{
public:
	ProjectSupplyQueryKeywords();
	ProjectSupplyQueryKeywords(const ProjectSupplyQueryKeywords &psqrk);	// copy constructor
	~ProjectSupplyQueryKeywords();
	void Clear();

	ProjectSupplyQueryKeywords& ProjectSupplyQueryKeywords::operator=(const ProjectSupplyQueryKeywords &psq);

	unsigned int m_track;
	vector<char *> v_keywords;
};

#endif // PROJECT_SUPPLY_QUERY_KEYWORDS_H