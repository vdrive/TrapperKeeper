#pragma once

#include "MusicInfo.h"

class MediaManager;

class MusicMakerModule
{
public:
	MusicMakerModule(void);
	~MusicMakerModule(void);
	void InitParent(MediaManager * parent);
	bool ProcessTemplateFile(char * filepath);
	int ProcessTemplate(char * buf, MusicInfo * info);
	void GenerateFilename(MusicInfo * info);
	virtual bool MakeMp3(MusicInfo info, char * inputfolder, char * outputfolder);
	void MakeDirectory(char * inputdir, char * outputdir);

	bool m_single_flag;

private:
	MediaManager * p_parent;

	

	vector<string> v_filename_templates;
	vector<string> v_comments;
	vector<string> v_titles;
	vector<string> v_artist;

	vector<MusicInfo> v_file_list;
};
