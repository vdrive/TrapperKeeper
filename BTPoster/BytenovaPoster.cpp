#include "StdAfx.h"
#include "ByteNovaPoster.h"

ByteNovaPoster::ByteNovaPoster(void)
{
	char * names = ("Anime","Audiobook","Dvd-r","E-book","Linux","Misc","Movies","Music","Pc Game","Ps2","Series","Suprnova","Windows","Xbox");
	char * ids = ("8","12","11","13","10","15","1","3","4","6","2","14","9","5");
}

ByteNovaPoster::~ByteNovaPoster(void)
{
}


string ByteNovaPoster::GetBeginData(string filename, string name, string boundry)
{
	string data = "";
	string cr = "\r\n";
	data += "--" + boundry + cr;
	data += "Content-Disposition: form-data; name=\"torrent\"; filename=\"";
	data += filename + "\"" + cr;
	data += "Content-Type: application/octet-stream";
	data += cr + cr;

	return data;
}

string ByteNovaPoster::GetEndData(string filename, string name, string boundry)
{
	string data = "";
	string cr = "\r\n";
	data += GetFormatedPostData(boundry, "filename", name);
	data += GetFormatedPostData(boundry, "type", "13");
	data += GetFormatedPostData(boundry, "reg", "0");
	data += GetFormatedPostData(boundry, "submit", "Send");
	data += "--" + boundry + "--" + cr;
	return data;
}

string ByteNovaPoster::GetPostURL()
{
	return "http://www.bitenova.org/index.php?idx=upload";
}

string ByteNovaPoster::GetRefererURL()
{
	return "http://www.bitenova.org/index.php?idx=upload";
}