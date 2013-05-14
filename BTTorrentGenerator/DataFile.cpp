#include "StdAfx.h"
#include "DataFile.h"
#include "BString.h"
#include "BInteger.h"

DataFile::DataFile(unsigned int len, string md5, string name, string path)
{
	fileLength = len;
	md5sum = md5;
	filename = name;

	b_len = new BInteger(len);

	if (md5.empty())
	{
		b_md5sum = NULL;
    }
	else
	{
		b_md5sum = new BString(md5sum);
	}

	if(!name.empty())
	{
		b_name = new BString(filename);
	}
	else
	{
		b_name = NULL;
	}

	v_datapath.push_back( new DataPath(path) );
}


DataFile::~DataFile()
{
	if(b_md5sum!= NULL)
		delete b_md5sum;
	if(b_name!= NULL)
		delete b_name;
	if(b_len!= NULL)
		delete b_len;
	vector <DataPath *>::iterator v_iter;
	for(v_iter = v_datapath.begin(); v_iter != v_datapath.end( ) ;v_iter++)
	{
		delete *v_iter;
	}
	v_datapath.clear();
}

int DataFile::getLength(){
	return fileLength;
}

vector<DataPath*> DataFile::get_v_datapath() {
	return v_datapath;
}

string DataFile::getFilename(){
	return filename;
}

int DataFile::GetFileLength(){
	return fileLength;
}

BInteger* DataFile::getBFileLen() {
	return b_len;
}

BString* DataFile::getBFilename() {
	return b_name;
}

BString* DataFile::getBMd5sum() {
	return b_md5sum;
}

string DataFile::getMd5sum(){
	return md5sum;
}