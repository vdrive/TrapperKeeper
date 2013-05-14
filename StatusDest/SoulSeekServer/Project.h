#pragma once

#define TRIGGER_WEIGHT		99

#include "SQLInterface.h"
#include "WeightedWords.h"
#include "Track.h"
#include <vector>
using namespace std;

class Project
{
public:
	Project(MYSQL* mysql, char* projectID);
	~Project(void);

	vector<string>* IsResult(vector<string>& searchString);


private:

	int project_id;
	string artist;
	string album;
	
	vector<WeightedWords> weighted_ex_keywords;
	vector<WeightedWords> weighted_keywords;
	vector<WeightedWords> killwords;

	vector<Track> tracks;
};
