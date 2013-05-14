#pragma once

#include "MediaSwarmerDlg.h"
#include "SwarmerSyncher.h"
#include "FileAssociation.h"
#include "../SamplePlugIn/dll.h"
#include "../DllLoader/AppID.h"
#include "IsoSwarmer.h"
#include "../tkcom/threadedobject.h"
#include <afxmt.h>

class MediaSwarmer : public Dll, public ThreadedObject
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
	bool OldInflateFile(char * filepath, char * outputdir);
	bool InflateFile(byte * data, CString filename, int Length);
	bool FillInChunk(CFile * outfile, unsigned int chunk_size);
	void ReadDestDir();
	void OldReadDestDir();
	void ExploreFolder(char * folder, vector<string> *filenames);
	void ReadMapsFile();
	bool AddFileAssociation(char * filename, char * assocname);

	void CheckMapsForDeletion(vector<TKSyncherMap::TKFile> &file_list);
	bool DeleteMappedFileList(vector<FileAssociation> vfa);
	void RemoveFileAssociationFromList(FileAssociation fa);
	void RewriteMapsFile();

private:
	HANDLE m_log_file;
	MediaSwarmerDlg m_dlg;
	MediaSyncher m_syncher;
	CCriticalSection m_lock;

	vector<FileAssociation> v_mapped_files;
public:
	UINT Run(void);
	void Log(const char* str);
};
