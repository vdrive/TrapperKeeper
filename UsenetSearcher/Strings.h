// Strings.h

#ifndef STRINGS_H
#define STRINGS_H

#include "ProjectDataStructure.h"

class Strings : public ProjectDataStructure
{
public:
	// Public Member Functions
	Strings();
	~Strings();

	void Clear();
	int GetBufferLength();
	int WriteToBuffer(char *buf);
	int ReadFromBuffer(char *buf);

	// Public Data Members
	vector<string> v_strings;
};

#endif // STRINGS_H