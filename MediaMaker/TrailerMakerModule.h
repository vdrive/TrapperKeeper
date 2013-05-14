#pragma once
#include "moviemakermodule.h"

class MediaManager;

class TrailerMakerModule :
	public MovieMakerModule
{
public:
	TrailerMakerModule(void);
	~TrailerMakerModule(void);

	void InitParent(MediaManager * parent);

	void MakeDirectory(char * inputfolder, char * outputfolder);
	void FindInputFilename();
	int CopyMovie(char * moviename,int size,char * outputfolder); 
	void ExploreFolder(char * folder, vector<string> * filenames);

private:
	MediaManager * p_parent;
	char m_inputname[256+1];
};
