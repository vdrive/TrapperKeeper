#pragma once
#include <string>
using namespace std;

class TrackerEntry
{
public:
	enum EntryType { URL, IP };

	TrackerEntry(void)
		: tracker(), bActive(true), eType(EntryType::URL)
	{
	}

	~TrackerEntry(void)
	{
	}

public:
	string		tracker;
	bool		bActive;
	EntryType	eType;
};
