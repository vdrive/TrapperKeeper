#include "StdAfx.h"
#include "project.h"

Project::Project(MYSQL* mysql, char* projectID)
{
	//Grab Project Information
	MYSQL_RES *res;
	SQLInterface db;

	string s;
	s = "SELECT artist, album FROM projects WHERE id = ";
	s.append(projectID);
	s = s + " LIMIT 1";
    res = db.Query(mysql,s);

	project_id = atoi(projectID);

	MYSQL_ROW row;

	row = mysql_fetch_row(res);
	artist = row[0];
	album = row[1];

	mysql_free_result(res);

	//Get Track names
	s = "select track_number, track_name from track_listing where project_id = ";
	s.append(projectID);
	res = db.Query(mysql,s);
	if( res != NULL )
	{
		while (row = mysql_fetch_row(res))
		{
			tracks.push_back(Track(artist,album,row[1], atoi(row[0])));
		}
	}

	mysql_free_result(res);

	//Get weighted_keywords
	s = "select keyword, weight from weighted_keywords where project_id = ";
	s.append(projectID);
	res = db.Query(mysql,s);
	if( res != NULL )
	{
		while (row = mysql_fetch_row(res))
		{
			weighted_keywords.push_back(WeightedWords(row[0], atoi(row[1])));
		}
	}

	mysql_free_result(res);

	//Get weighted_ex_keywords
	s = "select keyword, weight from weighted_ex_keywords where project_id = ";
	s.append(projectID);
	res = db.Query(mysql,s);
	if( res != NULL )
	{
		while (row = mysql_fetch_row(res))
		{
			weighted_ex_keywords.push_back(WeightedWords(row[0], atoi(row[1])));
		}
	}

	mysql_free_result(res);

	//Get killwords
	s = "select killword from killwords where project_id = ";
	s.append(projectID);
	res = db.Query(mysql,s);
	if( res != NULL )
	{
		while (row = mysql_fetch_row(res))
		{
			killwords.push_back(WeightedWords(row[0], TRIGGER_WEIGHT));
		}
	}

	mysql_free_result(res);

}

Project::~Project(void)
{
}

vector<string>* Project::IsResult(vector<string>& searchString)
{
	int score = 0;

	//TODO: Check for substrings
	//Check weighted keyword weights
	for(size_t j = 0; j < searchString.size(); j++)
	{
		for(size_t i = 0; i < weighted_keywords.size(); i++)
		{
			if(searchString[j][0] == '-')
			{
				int tempWeight = weighted_keywords[i].CompareWord(searchString[j].substr(1,searchString[j].size()-1));
				if(tempWeight != 0)
				{
					return NULL;  //We hit a killword...Return NULL
				}
			}
			else
			{
				int tempWeight = weighted_keywords[i].CompareWord(searchString[j]);
				if(tempWeight != 0)
				{
					score += tempWeight;
					break;
				}
			}
		}
		if(score >= TRIGGER_WEIGHT)
			break;
	}

	//Check weighted EXCLUSIVE keyword weights
	for(size_t j = 0; j < searchString.size(); j++)
	{
		for(size_t i = 0; i < weighted_ex_keywords.size(); i++)
		{
			if(searchString[j][0] == '-')
			{
				int tempWeight = weighted_ex_keywords[i].CompareWord(searchString[j].substr(1,searchString[j].size()-1));
				if(tempWeight != 0)
				{
					return NULL;  //We hit a killword...Return NULL
				}
			}
			else
			{
				int tempWeight = weighted_ex_keywords[i].CompareWord(searchString[j]);
				if(tempWeight != 0)
				{
					score += tempWeight;
					break;
				}
			}
		}
		if(score >= TRIGGER_WEIGHT)
			break;
	}

	//If we didnt meet the trigger weight, no point in proceeding
	if(score < TRIGGER_WEIGHT)
		return NULL;

	//Check kill words (Weightless)
	for(size_t j = 0; j < searchString.size(); j++)
	{
		for(size_t i = 0; i < killwords.size(); i++)
		{
			int tempWeight = killwords[i].CompareWord(searchString[j]);
			if(tempWeight != 0)
			{
				return NULL;
			}
		}
	}

	//TODO: Get filenames
	vector<string>* filenames = new vector<string>;

	for(size_t i = 0; i < tracks.size(); i++)
	{
		tracks[i].GetMatchingFileNames(searchString, filenames);
	}

	if(filenames->size() != 0)
        return filenames;

	return NULL;
}
