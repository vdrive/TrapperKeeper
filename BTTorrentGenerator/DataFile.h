#pragma once

#include "mysql.h"
#include "DBInterface.h"
#include "DataPath.h"

/* Each datafile in the torrent is represented by a DataFile object.
A DataFile object contains the metainfo of the data file.
A torrent(an object of type BTInput) usually contains a vector of
DataFiles*/

class BString;
class BInteger;
class DataFile
{
public:
	DataFile(unsigned int len, string md5, string name, string path);
	~DataFile();

	int getLength();
	string getMd5sum();
	string getFilename();
	int GetFileLength();
	
	BInteger * getBFileLen();
	BString * getBMd5sum();
	BString * getBFilename();
	vector<DataPath*> get_v_datapath();

protected:
	//File length in bytes
	unsigned int fileLength; 
	BInteger * b_len;
	BString * b_md5sum;
	BString * b_name;

	string md5sum;
	string filename;
	vector<DataPath*> v_datapath;

private:
};