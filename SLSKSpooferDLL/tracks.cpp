#include "stdafx.h"
#include "tracks.h"
#include <fstream>

tracks::tracks()
{
	getTracks();
}

tracks::~tracks()
{
}

void tracks::getTracks()
{
	int id;
	int num;
	char* tn;

	fstream fin("tracks.txt", ios::in);

	while(!fin.eof())
	{
		fin >> id >> num >> tn;
		track t;
		t.project_id = id;
		t.track_name = tn;
		t.track_number = num;
		Tracks.push_back(t);
	}
}