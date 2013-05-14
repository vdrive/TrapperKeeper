#pragma once
#include "datfilerecord.h"

class SwarmerDownloader;

class DatfileManager
{
public:
	DatfileManager(void);
	~DatfileManager(void);

	void InitParent(SwarmerDownloader * parent);

	string MakeSig2Dat(char * filename, char * hash, int filesize);
	void AsciiHashtoBinHash(char * hashin, char * hashout);
	void BinHashtoUUHash(char * hashin, char * hashout);
	void CreateNewDatfile(char * filename, char * hash, int filesize);
	bool CopyFile(char * fullpath, char * destpath);
	void RefreshDownloadFiles();
	DatfileRecord GetRecordFromFilename(char * filename);
	DatfileRecord GetRecordFromHash(char * hash);

	void ExploreFolder(char * folder, vector<string> *filenames);

	bool ReadCurrentDatfile();
	bool ReadWaitingDatfile();
	bool WriteCurrentDatfile();
	bool WriteWaitingDatfile();

private:
	
	SwarmerDownloader * p_parent;

	vector<DatfileRecord> v_current_datfiles;
	vector<DatfileRecord> v_waiting_datfiles;
};
