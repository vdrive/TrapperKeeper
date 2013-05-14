// stdafx.cpp : source file that includes just the standard includes
// SoulSeekClient.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

char* dupstring(const char * stringToCopy)
{
	char* returnString = new char[strlen(stringToCopy)+1];
	strcpy(returnString, stringToCopy);
	return returnString;
}