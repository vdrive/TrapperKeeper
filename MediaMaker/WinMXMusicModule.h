#pragma once
#include "musicmakermodule.h"

class WinMXMusicModule :
	public MusicMakerModule
{
public:
	WinMXMusicModule(void);
	~WinMXMusicModule(void);

	bool MakeMp3(MusicInfo info, char * inputfolder, char * outputfolder);
	void InitParent(MediaManager * parent);

private:
	MediaManager * p_parent;

};
