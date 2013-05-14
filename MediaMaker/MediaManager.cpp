#include "StdAfx.h"
#include "mediamanager.h"
#include "..\tksyncher\tksyncherinterface.h"


//
MediaManager::MediaManager(void)
{
}

MediaManager::~MediaManager(void)
{
}

void MediaManager::DllInitialize()
{
	// Create the dialog
	m_dlg.Create(IDD_MEDIA_DLG,CWnd::GetDesktopWindow());
	srand((unsigned int)time(NULL));
	// Init all of the modules controlled by the manager
	m_dlg.InitParent(this);
	m_movie_mod.InitParent(this);
	m_music_mod.InitParent(this);
	m_trailer_mod.InitParent(this);
	m_winmx_music_mod.InitParent(this);

	m_syncher.InitParent(this);
	m_deleter.InitParent(this);

	

}
void MediaManager::DllUnInitialize()
{

}


void MediaManager::DllStart()
{
	// Register the Syncher to start synching
	m_syncher.Register(this,"Media");
	DisplayInfo("Starting Syncher");


	//m_dlg.SetTimer(1,60*1000,NULL);
	m_dlg.SetTimer(1,6*60*60*1000,NULL);
	//We used to remove the Kazaa database file so it could be rebuilt.
/*
	char removefilebase[256];
	char deletefilename[256];
	strcpy(removefilebase,"C:\\Onsystems");
	char intchar[3];
	int number = 0;
	char mediainfo[4000];


	for (int i = 0; i<100; i++)
	{
		strcpy(deletefilename,removefilebase);
		_itoa(i,intchar,10);
		strcat(deletefilename,intchar);
		strcat(deletefilename,"\\Kazaa Lite K++\\db\\data256.dbb");
		remove(deletefilename);
		sprintf(mediainfo,"Removing datfile %s",deletefilename);
		DisplayInfo(mediainfo);
	}
*/
}

void  MediaManager::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	DisplayInfo("Starting Display");

	//Debugging
//	m_music_mod.MakeDirectory("C:\\syncher\\rcv\\Media.music\\2003-07-02 - Eagle Eye Cherry  (Universal)\\","C:\\FastTrack Shared\\Music\\Test\\");
//	m_music_mod.MakeDirectory("F:\\SHARED\\2004-01-06 - Barry\\","C:\\FastTrack Shared\\Music\\Test\\");

	//m_movie_mod.MakeDirectory("C:\\syncher\\rcv\\Media.movies\\Test Movie 2\\","C:\\FastTrack Shared\\Movies\\Test\\");

//	m_movie_mod.MakeDirectory("C:\\syncher\\rcv\\Media.movies\\Test Movie 2\\","C:\\FastTrack Shared\\Movies\\Test\\");
//	m_movie_mod.MakeDirectory("C:\\syncher\\rcv\\Media.movies\\Test Movie 2\\","C:\\FastTrack Shared\\Movies\\Test\\");
//	m_movie_mod.MakeDirectory("C:\\syncher\\rcv\\Media.movies\\Test Movie\\","C:\\FastTrack Shared\\Movies\\Test\\");
//	m_movie_mod.MakeDirectory("C:\\syncher\\rcv\\Media.movies\\Test Movie 2\\","C:\\FastTrack Shared\\Movies\\Test\\");
	//m_trailer_mod.MakeDirectory("C:\\syncher\\rcv\\Media.movies\\The Calcium Kid\\","C:\\FastTrack Shared\\Trailers\\Test\\");
	
//	m_winmx_music_mod.MakeDirectory("F:\\SHARED\\Trapper Keeper Media Test\\ADAM SANDLER\\","C:\\FastTrack Shared\\WinMXMusic\\Test\\");
//	m_winmx_music_mod.MakeDirectory("F:\\SHARED\\Trapper Keeper Media Test\\JOSH GROBAN\\","C:\\FastTrack Shared\\WinMXMusic\\Test\\");
//	m_winmx_music_mod.MakeDirectory("F:\\SHARED\\Trapper Keeper Media Test\\MY CHEMICAL ROMANCE\\","C:\\FastTrack Shared\\WinMXMusic\\Test\\");


}
//
//
//

void MediaManager::DisplayInfo(char * info)
{
	CFile logfile;
	if (logfile.Open("medialogfile.txt",CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate|CFile::shareDenyNone)== TRUE)
	{
		logfile.SeekToEnd();
		logfile.Write(info, (unsigned int)strlen(info));
		logfile.Write("\r\n",2);
		logfile.Close();
	}

	//Pass info onto the dialog.
	m_dlg.DisplayInfo(info);

	
}
//
//
//
void MediaManager::CompareMapsTemplates(vector<TKSyncherMap::TKFile> &template_list)
{
	ReadMapsFile();

	/*
	vector<TKSyncherMap::TKFile> template_copy;
	for (int i = 0; i < (int)template_list.size(); i++)
	{
		template_copy.push_back(template_list[i]);
	}
	*/
	//compare the template files from the maps vector
	vector<TKSyncherMap::TKFile>::iterator temp_iter = template_list.begin();
	// Iterate through the template list checking to see if they are in the maps file
	while (temp_iter != template_list.end())
	{
		for(int j = 0; j < (int)v_maps_list.size(); j ++)
		{
			// If the name of the template file is equal to a path in the maps file erase it
			if ((strcmp(temp_iter->m_name.c_str(), v_maps_list[j].m_path)) == 0)
			{
				template_list.erase(temp_iter);
				temp_iter --;
				break;
			}
		}
		temp_iter ++;
	}

	if (template_list.size() != 0)
	{
		//Pass the template list on to be processed
		KillGift();
		PreprocessTemplates(template_list);

		//When we're done restart WINMX and GIFT
		m_winmx_interface.RestartAllWinMx();
		RestartGift();
	}
}

//Reads the maps file and puts all entries on the vector
void MediaManager::ReadMapsFile()
{
	CFile list;
	FileAssociation fa;

	v_maps_list.clear();

	//  read each line of the file associations and place it onto the  vector.
	if( list.Open("maps.dat", CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone) == TRUE )
	{
		while(list.Read(&fa, sizeof(fa)) != 0)
		{
			v_maps_list.push_back(fa);

		}
		list.Close();
	}



}

//Determine which mod to send the template files to
void MediaManager::PreprocessTemplates(vector<TKSyncherMap::TKFile> &template_list)
{
	string filepath;
	char filedir[256+1]; // dir where the template file is
	char projectdir[256+1]; // dir where the project will end up
	CStdioFile tempfile;
	char buf[4096+1];
	int type = 0;

	char infostring[1024+1];

	// See if the logfile is too big and get rid of it if it is
	CheckLogfile();

	for (int i = 0; i < (int)template_list.size(); i++)
	{
		filepath = template_list[i].m_name;
		// Open the file template
		if (tempfile.Open(filepath.c_str(),CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone) == TRUE)
		{
			type = 0;
			//read in the template to find the type tag
			while (tempfile.ReadString(buf,sizeof(buf)-1)!=FALSE)
			{
				if (strstr(buf,"<KAZAA MUSIC>"))
				{
					type = 1;
					break;
				}
				else if (strstr(buf,"<KAZAA MOVIE>"))
				{
					type = 2;
					break;
				}

				else if (strstr(buf,"<KAZAA TRAILER>"))
				{
					type = 3;
					break;
				}

				else if (strstr(buf,"<WINMX MUSIC>"))
				{
					type = 4;
					break;

				}
				else if (strstr(buf,"<WINMX MOVIE>"))
				{
					type = 2;
					break;
				}

			}
			tempfile.Close();
		}
		// strip off everything after the last \ to determine the basedir
		strcpy(filedir,filepath.c_str());
		char * ptr;
		ptr = strrchr(filedir,'\\');
		*ptr = '\0';
		// strip back one more \ to find the project directory
		ptr = strrchr(filedir,'\\');

		sprintf(infostring,"Type %i ",type);
		DisplayInfo(infostring);

		switch (type)
		{
			case 1:
			{
				// Base directory for the Music
				strcpy(projectdir,"C:\\FastTrack Shared\\Music");
				// Add what we determined the base directory for the project should be
				strcat(projectdir,ptr);
				// It's a directory
				strcat(projectdir,"\\");
				// Put back the \ for the file directory too
				strcat(filedir,"\\");
				m_music_mod.MakeDirectory(filedir,projectdir);
				sprintf(infostring,"Making dir %s",projectdir);
				DisplayInfo(infostring);
				break;
			}

			case 2:
			{
				//Base Directory for Kazaa Movies
				strcpy(projectdir,"C:\\FastTrack Shared\\Movies");
				strcat(projectdir,ptr);
				strcat(projectdir,"\\");
				strcat(filedir,"\\");
				m_movie_mod.MakeDirectory(filedir,projectdir);
				sprintf(infostring,"Making dir %s",projectdir);
				DisplayInfo(infostring);
				break;
			}

			case 3:
			{
				//Base Directory for Kazaa Trailer Movies
				strcpy(projectdir,"C:\\FastTrack Shared\\Trailers");
				strcat(projectdir,ptr);
				strcat(projectdir,"\\");
				strcat(filedir,"\\");
				m_trailer_mod.MakeDirectory(filedir,projectdir);
				sprintf(infostring,"Making dir %s",projectdir);
				DisplayInfo(infostring);
				break;
			}

			case 4:
			{
				// Base directory for the Music
				strcpy(projectdir,"C:\\FastTrack Shared\\WinMXMusic");
				// Add what we determined the base directory for the project should be
				strcat(projectdir,ptr);
				// It's a directory
				strcat(projectdir,"\\");
				// Put back the \ for the file directory too
				strcat(filedir,"\\");
				m_winmx_music_mod.MakeDirectory(filedir,projectdir);
				sprintf(infostring,"Making dir %s",projectdir);
				DisplayInfo(infostring);
				break;
			}

			default:
			{
				DisplayInfo("Unknown Template, a problem occured");
				break;
			}

		}

	}

	DisplayInfo("All templates completed exiting normally");

}

//
//
//

// Adds a file association to the maps file
bool MediaManager::AddFileAssociation(char * filename, char * assocname)
{
	// Create a file association for the two files and save it to the maps file
	FileAssociation fa;
	strcpy(fa.m_path, filename);
	strcpy(fa.m_assoc_path, assocname);
	// if you were able to save the file association to the maps file
	if(fa.Save() == true)
	{
//		char buf[1024+1];
//		sprintf(buf,"Created File %s from file %s",assocname,filename);
//		DisplayInfo(buf);
		return true;
	}
	return false;
}

//
//
// Write out the current vector of file over the current maps file
void MediaManager::RewriteMapsFile()
{
	// Delete the old maps file
	remove("maps.dat");
	//go through the vector of current maps and write it out to the file
	for (int i = 0; i< (int)v_maps_list.size(); i ++)
	{
		v_maps_list[i].Save();
	}
}

//
//
//
void MediaManager::CheckMapsForDeletion(vector<TKSyncherMap::TKFile> &file_list)
{
	int found = 0;
	vector<FileAssociation> files_for_deletion; //List of Files to delete
	char buf[1024+1];

	ReadMapsFile();
	//Step through each entry in the maps file
	for (int i = 0; i < (int) v_maps_list.size(); i ++)
	{
		//Compare each maps entry to the Syncher Maps
		for (int j = 0; j < (int)file_list.size(); j++)
		{
			// For debugging 
			//char mapslist[256+1];
			//char filelist[256+1];
			//strcpy(mapslist,v_maps_list[i].m_path);
			//strcpy(filelist,file_list[j].m_name.c_str());
			//if (strcmp(mapslist, filelist) == 0)
			if (stricmp(v_maps_list[i].m_path, file_list[j].m_name.c_str()) == 0)
			{
				// we found it
				sprintf(buf,"Found Compare %s to %s ",v_maps_list[i].m_path,file_list[j].m_name.c_str());
				DisplayInfo(buf);
				found = 1;
				break;
			}


		}
		if (found == 0)
		{
			// we didn't find it so it is supposed to be deleted
			files_for_deletion.push_back(v_maps_list[i]);
		}
		// reset found flag
		found = 0;
	}
	// Delete all of the files that are supposed to be deleted
	if (files_for_deletion.size() > 0)
	{
		m_deleter.DeleteMappedFileList(files_for_deletion);
	}
}

//
//
//

void MediaManager::RemoveFileAssociationFromList(FileAssociation fa)
{
	vector<FileAssociation>::iterator fa_iter = v_maps_list.begin();
	while (fa_iter != v_maps_list.end())
	{
		if ((strcmp(fa.m_assoc_path,fa_iter->m_assoc_path) == 0)&&(strcmp(fa.m_path,fa_iter->m_path) == 0))
		{
			v_maps_list.erase(fa_iter);
			break;
		}
		fa_iter ++;
	}
}


//
//
//

// Kazaa Interface functions
int MediaManager::GetNumKazaas()
{
	return (m_kazaa_interface.RequestKazaaNumber());
}

bool MediaManager::KillAllKazaa()
{
	return(m_kazaa_interface.KillAllKazaa());
}

bool MediaManager::RelaunchAllKazaa()
{
	return(m_kazaa_interface.ResumeLaunchingKazaa());
}

bool MediaManager::KillGift()
{
	return(m_gift_interface.StopSharing());
}

bool MediaManager::RestartGift()
{
	return(m_gift_interface.StartSharing());
}

bool MediaManager::KillWinMx()
{
	return(m_winmx_interface.StopRunningWinMx());
}

bool MediaManager::RestartWinMx()
{
	return(m_winmx_interface.RestartAllWinMx());
}

void MediaManager::CheckLogfile()
{
	int size;
	CFile logfile;

	if (logfile.Open("medialogfile.txt",CFile::modeRead)== TRUE)
	{
		size = (int)logfile.GetLength();
		logfile.Close();
		if (size > 10000000)
		{
			remove("medialogfile.txt");
		}
	}

}

void MediaManager::TimerFired()
{
	bool restart = false;
	restart = m_deleter.DeleteExtraFolders();

	if (restart == true)
	{
		RestartGift();
		RestartWinMx();
	}
}