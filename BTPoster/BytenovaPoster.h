#pragma once
#include "poster.h"

class ByteNovaPoster :
	public Poster
{
public:
	ByteNovaPoster(void);
	~ByteNovaPoster(void);

	string GetBeginData(string filename, string name, string boundry);
	string GetEndData(string filename, string name, string boundry);
	string GetPostURL();
	string GetRefererURL();
};
