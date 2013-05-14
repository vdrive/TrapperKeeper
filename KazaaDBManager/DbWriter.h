#pragma once
#include "RecordHeader.h"
#include "Record.h"
#include"..\\KazaaDbManager\\FileMetaData.h"


class DbWriter
{
public:
	DbWriter(void);
	~DbWriter(void);

	void WriteDbFile(vector <FileMetaData> files);
	void WriteKnownFile(FileMetaData metadata);
	void TestWriting();
	void ExploreFolder(char * folder, vector<string>  *filenames);
	void CalculateHash(char * outputfilename, char * hashin);
	int ReadEntry(CStdioFile * dbfile, FileMetaData * mdata, int entrysize);
	void ReadInMovieMetadata();
	void RetrieveEntry(byte * hash);
	void CopyDatabases();
	bool CopyDatFile(char * infile, char * outfile);
	void ClearDatabases();
	void WriteMovieFile(FileMetaData metadata, RecordHeader header);

private:
	vector<FileMetaData> v_movie_entries;

};
