#include "stdafx.h"
#include "spoofinfo.h"
#include <fstream>

spoofinfo::spoofinfo()
{
	getSpoofInfo();
}

spoofinfo::~spoofinfo()
{
}

void spoofinfo::getSpoofInfo()
{
	int id, tn, ts, tbr;

	fstream fin("spoofinfo.txt", ios::in);

	while(!fin.eof())
	{
		fin >> id >> tn >> ts >> tbr;
		spoof s;
		s.project_id = id;
		s.track_number = tn;
		s.size = ts;
		s.bitrate = tbr;
		Spoofs.push_back(s);
	}
}