#include "StdAfx.h"
#include "musicmakermodule.h"
#include "MediaManager.h"
#include "FrameHeader.h"
#include "ID3v1Tag.h"
#include "ID3v2Tag.h"
#include "imagehlp.h"
#include <io.h>

MusicMakerModule::MusicMakerModule(void)
{
	m_single_flag = false;
}

MusicMakerModule::~MusicMakerModule(void)
{
}


void MusicMakerModule::InitParent(MediaManager *parent)
{
	p_parent = parent;	
}

//
//
//

bool MusicMakerModule::ProcessTemplateFile(char * filepath)
{
	CStdioFile templatefile;
	string template_filename;
	char buf[4096+1];
	int ret;

	MusicInfo info;
	template_filename = filepath;
	template_filename += "template.mak";



	if (templatefile.Open(template_filename.c_str(),CFile::typeText|CFile::modeRead|CFile::shareDenyWrite)==FALSE)
	{
		return 0;
	}

	v_filename_templates.clear();
	v_comments.clear();
	//loop to read in and process each line.
	while(templatefile.ReadString(buf,sizeof(buf)-1)!=FALSE)
	{
		ret = ProcessTemplate(buf, &info);
		//return value of 2 means we read in a title and therefore are finished with the info for one song
		if (ret == 2)
		{
			// put the song on the vector of files to create
			v_file_list.push_back(info);
		}
	}
	
	// If we have a single put 9 more copies onto the vector
	if (m_single_flag == true)
	{
		for(int i = 1; i <= 9; i++) 
		{
			info.m_single_offset = i;
			v_file_list.push_back(info);
		}
	}


	
	// iterate through the vector and generate the filenames
	vector<MusicInfo>::iterator data_iter = v_file_list.begin();
	while (data_iter != v_file_list.end())
	{
		GenerateFilename(&(*data_iter));
		data_iter ++;	
	}	
		
	return true;
}

//
//
//

int MusicMakerModule::ProcessTemplate(char * buf, MusicInfo * info)
{
	CString bufstring;
	string tempstring;
	// check each line for the template tag and put the data into the correct spot
	// Fill the vectors for the randomized parts first
	if (strstr(buf,"<filename template>"))
	{
		bufstring = buf + strlen("<filename template>");
		bufstring.TrimLeft();
		bufstring.TrimRight();
		tempstring = bufstring;
		v_filename_templates.push_back(tempstring);
		return 1;
	}
	else if (strstr(buf,"<comment>"))
	{
		bufstring = buf + strlen("<comment>");
		bufstring.TrimLeft();
		bufstring.TrimRight();
		tempstring = bufstring;
		v_comments.push_back(tempstring);
		return 1;
	}
	else if (strstr(buf,"<title>"))
	{
		bufstring = buf + strlen("<title>");
		bufstring.TrimLeft();
		bufstring.TrimRight();
		int rnum = rand()%10;
		char *buf = new char[strlen(bufstring)+1];
		strcpy(buf,bufstring);
		switch (rnum)
		{
			case 1:
			{
				strlwr(buf);
			}
			case 2: 
			{
				//This keeps changing every time a customer decides they like or hate caps
				strupr(buf);
			}
			case 3:
			{
				strlwr(buf);
			}
			case 4:
			{
				strlwr(buf);
			}
			default:
			{

			}
		}
		info->m_title = buf;
		delete [] buf;

		return 2;
	}
	else if (strstr(buf,"<artist>"))
	{
		bufstring = buf + strlen("<artist>");
		bufstring.TrimLeft();
		bufstring.TrimRight();
		info->m_artist = bufstring;
		return 1;
	}
	else if (strstr(buf,"<album>"))
	{
		bufstring = buf + strlen("<album>");
		bufstring.TrimLeft();
		bufstring.TrimRight();
		info->m_album = bufstring;
		return 1;
	}
	else if (strstr(buf,"<track>"))
	{
		bufstring = buf + strlen("<track>");
		bufstring.TrimLeft();
		bufstring.TrimRight();
		info->m_track = bufstring;
		return 1;
	}
	else if (strstr(buf,"<genre>"))
	{
		bufstring = buf + strlen("<genre>");
		bufstring.TrimLeft();
		bufstring.TrimRight();
		info->m_genre = bufstring;
		return 1;
	}
		else if (strstr(buf,"<single>"))
	{
		m_single_flag = true;
		return 1;
	}
	else if (strstr(buf,"<year>"))
	{
		bufstring = buf + strlen("<year>");
		bufstring.TrimLeft();
		bufstring.TrimRight();
		info->m_year = bufstring;
		return 1;
	}
	return 0;
}

void MusicMakerModule::GenerateFilename(MusicInfo * info)
{
	string filename_template;
	string filename;

	bool replace_spaces_with_underscores=false;
	bool replace_spaces_with_dashes=false;

	int rnum = (rand()%(int)v_filename_templates.size());
	filename_template=v_filename_templates[rnum];
	if (v_comments.size() > 0)
	{
		rnum = (rand()%(int)v_comments.size());
		info->m_comments= v_comments[rnum];
	}
	// Make a buffer to hold the template so we can walk through it
	char *tempbuf=new char[filename_template.size()+1];
	strcpy(tempbuf,filename_template.c_str());
	char *ptr=tempbuf;


    // at each position see if we are at one of the tags
	while(*ptr!='\0')
	{
		// If we are at a normal tag replace it with the name from info
		if(strstr(ptr,"[Artist]")==ptr)
		{
			filename+=info->m_artist.c_str();
			ptr+=strlen("[Artist]");
		}
		// if we are at a lower case tag, lower the name from info then replace
		else if(strstr(ptr,"[artist]")==ptr)
		{
			char *buf=new char[info->m_artist.size()+1];
			strcpy(buf,info->m_artist.c_str());
			strlwr(buf);
			filename+=buf;
			delete [] buf;
			ptr+=strlen("[artist]");
		}
		//If we are at an upper case tag, upper case the name then replace
		else if(strstr(ptr,"[ARTIST]")==ptr)
		{
			char *buf=new char[info->m_artist.size()+1];
			strcpy(buf,info->m_artist.c_str());
			strupr(buf);
			filename+=buf;
			delete [] buf;
			ptr+=strlen("[ARTIST]");
		}
		else if(strstr(ptr,"[Album]")==ptr)
		{
			filename+=info->m_album.c_str();
			ptr+=strlen("[Album]");
		}
		else if(strstr(ptr,"[album]")==ptr)
		{
			char *buf=new char[info->m_album.size()+1];
			strcpy(buf,info->m_album.c_str());
			strlwr(buf);
			filename+=buf;
			delete [] buf;
			ptr+=strlen("[album]");
		}
		else if(strstr(ptr,"[ALBUM]")==ptr)
		{
			char *buf=new char[info->m_album.size()+1];
			strcpy(buf,info->m_album.c_str());
			strupr(buf);
			filename+=buf;
			delete [] buf;
			ptr+=strlen("[ALBUM]");
		}
		else if(strstr(ptr,"[Title]")==ptr)
		{
			filename+=info->m_title.c_str();
			ptr+=strlen("[Title]");
		}
		else if(strstr(ptr,"[title]")==ptr)
		{
			char *buf=new char[info->m_title.size()+1];
			strcpy(buf,info->m_title.c_str());
			strlwr(buf);
			filename+=buf;
			delete [] buf;
			ptr+=strlen("[title]");
		}
		else if(strstr(ptr,"[TITLE]")==ptr)
		{
			char *buf=new char[info->m_title.size()+1];
			strcpy(buf,info->m_title.c_str());
			strupr(buf);
			filename+=buf;
			delete [] buf;
			ptr+=strlen("[TITLE]");
		}
		else if((strstr(ptr,"[Track]")==ptr) || (strstr(ptr,"[track]")==ptr) || (strstr(ptr,"[TRACK]")==ptr))
		{
			filename+=info->m_track.c_str();
			ptr+=strlen("[Track]");
		}
		else if(strstr(ptr,"[underscore spaces]")==ptr)
		{
			replace_spaces_with_underscores=true;
			ptr+=strlen("[underscore spaces]");
		}
		else if(strstr(ptr,"[dash spaces]")==ptr)
		{
			replace_spaces_with_dashes=true;
			ptr+=strlen("[dash spaces]");
		}
		//No tag just put whatever character you find into the name
		else	
		{
			char c=*ptr;
			filename+=c;
			ptr++;
		}
	}
	// Add the file extension
	filename+=".mp3";
	//if you are tagged to underscore spaces step through the string replacing spaces with underscores
	if(replace_spaces_with_underscores)
	{
		char *buf=new char[filename.size()+1];
		strcpy(buf,filename.c_str());
		char *ptr=buf;
		while(*ptr!='\0')
		{
			if(*ptr==' ')
			{
				*ptr='_';
			}
			ptr++;
		}
		filename=buf;
		delete [] buf;
	}
	// Same deal with dashed spaces
	else if(replace_spaces_with_dashes)
	{
		char *buf=new char[filename.size()+1];
		strcpy(buf,filename.c_str());
		char *ptr=buf;
		while(*ptr!='\0')
		{
			if(*ptr==' ')
			{
				*ptr='-';
			}
			ptr++;
		}
		filename=buf;
		delete [] buf;
	}

	info->m_filename = filename;

	delete [] tempbuf;
}

//
//
//

bool MusicMakerModule::MakeMp3(MusicInfo info, char * inputfolder, char * outputfolder)
{

	CFile input_file;
	CFile output_file;

	char msg[1024];
	char outputfilename[256+1];
	char inputfilename[256+1];

	//Set up the outputfile
	strcpy(outputfilename, outputfolder);
	strcat(outputfilename,info.m_filename.c_str());

	//If the filename appears to be too long, cut it off and add the .mp3 tag so at least it works
	if (strlen(outputfilename) >= 255)
	{
		outputfilename[252] = '\0';
		strcat(outputfilename,".mp3");
	}

	// Create the input filename with the folder plus input plus the track number
	strcpy(inputfilename,inputfolder);
	strcat(inputfilename,"Input");
	strcat(inputfilename,info.m_track.c_str());
	strcat(inputfilename,".mp3");

	//Try and open the input file
	if(input_file.Open(inputfilename,CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone)==FALSE)
	{
		DWORD Damnerror = ::GetLastError();
		sprintf(msg,"Failed to open input file %s",inputfilename);
		p_parent->DisplayInfo(msg);
		return false;
	}

	MakeSureDirectoryPathExists(outputfilename);
	// see if the file exists already
	_finddata_t data;
	intptr_t handle;
//	while (output_file.Open(outputfilename,CFile::modeRead) == TRUE)
	while ((handle = _findfirst(outputfilename, &data)) != -1)
	{
		_findclose(handle);
		//if it exists re-generate the filename and try again
		GenerateFilename(&info);
		strcpy(outputfilename, outputfolder);
		strcat(outputfilename,info.m_filename.c_str());
		//output_file.Close();
	}
	//Create the output file
	if(output_file.Open(outputfilename,CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate|CFile::typeBinary|CFile::shareDenyWrite)==FALSE)
	{
		DWORD Damnerror = ::GetLastError();
		sprintf(msg,"Failed to create output file %s",outputfilename);
		p_parent->DisplayInfo(msg);
		return false;
	}

	ID3v2Tag id3v2_tag;
	// We don't like V2 tags so if you see it get rid of it
	if(id3v2_tag.ExtractTag(&input_file)==false)
	{
		// If there was no tag, rewind the file
		input_file.SeekToBegin();
	}

	//Get the frame Header
	FrameHeader frame_header;
	frame_header.ExtractHeader(&input_file);

	unsigned int bitrate=frame_header.ReturnBitrate();

	// Put back the frame header
	input_file.Seek(0-sizeof(FrameHeader),CFile::current);

	// We're gonna calculate the frame count based on the time and the bitrate
	unsigned int frame_count=0;

	// Need to be able to round up if necessary
	int seconds = 300;  // begin with a 5 minute song, will be resized later
	double dividend=1*1*bitrate*seconds;
	double divisor=frame_header.ReturnFrameSize()*8*1;
	double quotient=dividend/divisor;

	frame_count=(unsigned int)quotient;

	quotient-=(unsigned int)quotient;
	if(quotient>0.44444444444444444444444444444444)
	{
		frame_count++;
	}	

	// Add frames of silence...up to 3 seconds worth
	unsigned int pre_silence_frame_count=0;

	dividend=1*1*bitrate*3;
	divisor=frame_header.ReturnFrameSize()*8*1*1;
	quotient=dividend/divisor;
	
	pre_silence_frame_count=(unsigned int)quotient;

	quotient-=(unsigned int)quotient;
	if(quotient>0.44444444444444444444444444444444)
	{
		pre_silence_frame_count++;
	}

	// Randomly pick up to 3 seconds worth of frames
	pre_silence_frame_count=rand()%pre_silence_frame_count;

	bool is_first_frame_of_file=true;

	bool is_padded=false;
	for(unsigned int i=0;i<pre_silence_frame_count;i++)
	{
		frame_header.IsPadded(is_padded^=true);		// toggle the padded bit

		output_file.Write(&frame_header,sizeof(FrameHeader));

		unsigned char *buf=new unsigned char[frame_header.ReturnFrameSize()-sizeof(FrameHeader)];
		
		for(unsigned int j=0;j<frame_header.ReturnFrameSize()-sizeof(FrameHeader);j++)
		{
			buf[j]=rand()%10;	// 0-9 is ever more "more or less silence"
		}

		if(is_first_frame_of_file)
		{
			is_first_frame_of_file=false;
			unsigned int deadbeef=0xDEADBEEF;
			memcpy(buf,&deadbeef,sizeof(deadbeef));
		}
		
		output_file.Write(buf,frame_header.ReturnFrameSize()-sizeof(FrameHeader));
		delete [] buf;
	}


	// Adjust for pre-silence frame count frames
	frame_count-=pre_silence_frame_count;

	// Begin again
	input_file.SeekToBegin();
	// We don't like V2 tags so if you see it get rid of it
	if(id3v2_tag.ExtractTag(&input_file)==false)
	{
		// If there was no tag, rewind the file
		input_file.SeekToBegin();
	}
	// input_file.SeekToBegin();
	// Create the new file from the old one
	for(i=0;i<frame_count;i++)
	{
		frame_header.ExtractHeader(&input_file);

		unsigned int frame_size=frame_header.ReturnFrameSize()-sizeof(FrameHeader);
	// Problem here FUCK
		if (frame_size > 2000000)
		{
			frame_size = 414;
		}
		unsigned char *buf=new unsigned char[frame_size];
		
		// If we've reached the end of the file, then rewind it or set it to silence
		if(input_file.Read(buf,frame_size)==NULL)
		{
			for(unsigned int j=0;j<frame_size;j++)
			{
				buf[j]=rand()%10;	// 0-9 is "more or less silence"
			}
		}

		output_file.Write(&frame_header,sizeof(FrameHeader));
		output_file.Write(buf,frame_size);

		delete [] buf;
	}

	int modlen = 0;
	int flen = (int)output_file.GetLength();
	// If this is flagged as a single we assign it a specific filesize so that each file is > 200k apart
	if (m_single_flag)
	{

		//Correct way 
		
		double twenty_five_percent=((double)flen)/4.0;	// on a 4 MB file, this is 1 MB
		//double twenty_percent_of_the_twenty_five_percent=twenty_five_percent/5.0;	// on a 4 MB file, this is 200 KB

		// on a 4 MB file, this is 4 MB - 1 MB + (300 KB * n) where n = single_offset
		int delta=(int)(((-1.0)*twenty_five_percent)+((300*1024)*info.m_single_offset));


		int noise=rand() % (5*1024);	// up to 5K of noise


		output_file.SetLength(flen+delta+noise);	// delta is -1 MB and up (in the positive direction) for a 4 MB file + up to 5K of noise
		
		/*
		// The Warner Special Trial way, don't use this normally
		int randval=rand() | (rand()<<16);	// make 32-bit rand (sort of...the bits will be 0xxx:xxxx:0xxxx:xxxx with the x's being random)

		int delta=(randval % (15000000));	// delta = rand within 6 meg or so

		output_file.SetLength(1000000+delta);
		*/
	}
	//If it isn't a single give it a random filesize from about 1 to 6 megs
	else
	{

			// Resize the file size +/- 50% (on a 4 MB file, this is +/- 1 MB)
			double fifty_percent=((double)flen)/2.0;	// on a 4 MB file, this is 2 MB
	
			int randval=rand() | (rand()<<16);	// make 32-bit rand (sort of...the bits will be 0xxx:xxxx:0xxxx:xxxx with the x's being random)

			int delta=(randval % (7000000));	// delta = rand within 6 meg or so

			output_file.SetLength(1000000+delta);


 
	}

	// add the length of the file plus the length of the tag to determine the final file size 
	flen = (int)(output_file.GetLength()+sizeof(ID3v1Tag));



	//add the inverse of the filesize mod 137 so the final size will be divisible by 137
	modlen = (flen%137);
	modlen = 137 - modlen;
	unsigned char *buffer=new unsigned char[modlen];
	for(int m=0;m<modlen;m++)
	{
		buffer[m]=rand()%10;	
	}
	output_file.Write(buffer,modlen);		

	// ID3v1 Tag
	ID3v1Tag tag;

	tag.Init();
	tag.SetAlbum((char *)info.m_album.c_str());
	tag.SetArtist((char *)info.m_artist.c_str());
	tag.SetGenre(atoi((char *)info.m_genre.c_str()));
	tag.SetYear((char *)info.m_year.c_str());
	tag.SetComment((char *)info.m_comments.c_str());

	tag.SetTitle((char *)info.m_title.c_str());

	output_file.Write(&tag,sizeof(ID3v1Tag));


	//Clean Up
	tag.Clear();

	delete [] buffer;

	input_file.Close();
	output_file.Close();

	// Add the file association 
	if (p_parent->AddFileAssociation(inputfilename, outputfilename) == true)
	{
		 if (m_single_flag == false)
		 {
			//file association worked so remove the input if it is not a single
			CFile::Remove(inputfilename);
		 }		
			return true;
	}
	return false;

}

void MusicMakerModule::MakeDirectory(char * inputdir, char * outputdir)
{

	bool success;
	// Get rid of any leftover info from previous makes
	v_file_list.clear();
	m_single_flag = false;
	srand((unsigned int)time(NULL));
	// Process the template file to create the info vector
	ProcessTemplateFile(inputdir);
	for (int i = 0; i < (int)v_file_list.size(); i++)
	{
		success = MakeMp3(v_file_list[i],inputdir, outputdir);

	}
	// Put the Template into the maps file
	char inputfa[256+1];
	strcpy(inputfa,inputdir);
	strcat(inputfa,"template.mak");
	p_parent->AddFileAssociation(inputfa, "");

}