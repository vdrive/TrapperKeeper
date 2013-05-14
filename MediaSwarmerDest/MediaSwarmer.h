#pragma once

#include "MediaSwarmerDlg.h"
#include "SwarmerSyncher.h"
#include "FileAssociation.h"
#include "../SamplePlugIn/dll.h"
#include "../DllLoader/AppID.h"

class MediaSwarmer : public Dll
{
public:
	MediaSwarmer(void);
	~MediaSwarmer(void);

	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();

	void DataReceived(char *source_name, void *data, int data_length);
	bool DeflateFile(char * filepath);
	bool InflateFile(char * filepath, char * outputdir);
	bool FillInChunk(CFile * outfile, unsigned int chunk_size);
	void ReadDestDir();
	void ExploreFolder(char * folder, vector<string> filenames);
	void ReadMapsFile();
	bool AddFileAssociation(char * filename, char * assocname);

	void CheckMapsForDeletion(vector<TKSyncherMap::TKFile> &file_list);
	bool DeleteMappedFileList(vector<FileAssociation> vfa);
	void RemoveFileAssociationFromList(FileAssociation fa);
	void RewriteMapsFile();

private:
	MediaSwarmerDlg m_dlg;
	MediaSyncher m_syncher;

	vector<FileAssociation> v_mapped_files;
};
