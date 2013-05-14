// wma generator.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "wma generator.h"
#include "MainFrm.h"

#include "ChildFrm.h"
#include "wma generatorDoc.h"
#include "wma generatorView.h"
#include <process.h>
#include <fstream>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define MAX_LOADSTRING 100
#define NUM_OF_INPUTS 100

vector<string> v_filename_templates;
vector<string> v_comments;
vector<string> v_titles;
vector<string> v_artist;
bool m_single_flag;
vector<MusicInfo> v_file_list;


// CdeletemeApp

BEGIN_MESSAGE_MAP(CwmageneratorApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()


// CdeletemeApp construction

CwmageneratorApp::CwmageneratorApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CdeletemeApp object

CwmageneratorApp theApp;

// CdeletemeApp initialization

BOOL CwmageneratorApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)
	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_wmageneratorTYPE,
		RUNTIME_CLASS(CwmageneratorDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CwmageneratorView));
	AddDocTemplate(pDocTemplate);
	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;
	// call DragAcceptFiles only if there's a suffix
	//  In an MDI app, this should occur immediately after setting m_pMainWnd
	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// The main window has been initialized, so show and update it
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();
	Runthis();
	return TRUE;
}



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CwmageneratorApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CdeletemeApp message handlers



int Runthis()
{
	srand((unsigned int)time(NULL));
	
	CStdioFile templatefile;
	string template_filename = "";
	char buf[4096+1];
	int ret;

	MusicInfo info;
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
	/*vector<MusicInfo>::iterator data_iter = v_file_list.begin();
	while (data_iter != v_file_list.end())
	{
		GenerateFilename(&(*data_iter));
		data_iter ++;	
	}*/

	RenameInputs();
		
	return 0;
}

int ProcessTemplate(char * buf, MusicInfo * info)
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
		//check for redundant file templates
		bool found = false;
		for (unsigned int i=0; i < v_filename_templates.size(); i++) {
			if (tempstring == v_filename_templates[i]) {
				found = true;
				break;
			}
		}
		if (!found) {
			v_filename_templates.push_back(tempstring);
		}
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

void GenerateFilename(MusicInfo * info)
{
	//this function creates a file for every filename in the template
	//it choose a random input###.wma file to copy

	string filename_template;
	string filename;

	bool replace_spaces_with_underscores=false;
	bool replace_spaces_with_dashes=false;

	//int rnum = (rand()%(int)v_filename_templates.size());
	//filename_template=v_filename_templates[rnum];
	

	for (int i=0; i < (int)v_filename_templates.size(); i++) {
		filename="";
		filename_template=v_filename_templates[i];
		char *tempbuf=new char[filename_template.size()+1];
		if (v_comments.size() > 0)
		{
			int rnum = (rand()%(int)v_comments.size());
			info->m_comments= v_comments[rnum];
		}
		// Make a buffer to hold the template so we can walk through it
		
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
		delete [] tempbuf;
		// Add the file extension
		filename+=".wma";
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

		string filename2 = "input";
		int num = rand()%(int)NUM_OF_INPUTS;
		num++;
		char tempnum[5];
		if (num >=100) {
			filename2 += itoa(num, tempnum, 10);
		}
		else if ((num < 100) && (num >= 10)) {
			filename2 += "0";
			filename2 += itoa(num, tempnum, 10);
		}
		else if (num < 10) {
			filename2 += "00";
			filename2 += itoa(num, tempnum, 10);
		}
		filename2 += ".wma";

		filename2.insert(0, ".\\inputs\\");
		filename.insert(0, ".\\output\\");

		fstream inputfile, outputfile;
		inputfile.open(filename2.c_str(), fstream::in | fstream::binary);
		bool temp2 = inputfile.is_open();
		outputfile.open(filename.c_str(), fstream::out | fstream::binary);
		bool temp3 = outputfile.is_open();

		char buffer[4096];
		DWORD dwRead;
		
		do
		{
			inputfile.read(buffer, 4096);
			dwRead = inputfile.gcount();
			outputfile.write(buffer, dwRead);
		}
		while (dwRead > 0);

		inputfile.close();
		outputfile.close();
	}
}

void RenameInputs()
{
	//this function is a variation of Generate filenames
	//it will create as many files as there are inputs, with a generated filename
	
	string filename_template;
	string filename;

	bool replace_spaces_with_underscores=false;
	bool replace_spaces_with_dashes=false;

	//int rnum = (rand()%(int)v_filename_templates.size());
	//filename_template=v_filename_templates[rnum];
	

	for (int i=0; i < NUM_OF_INPUTS; i++) {
		filename="";
		int num = int(v_filename_templates.size() *(   (double)rand() / ((double)(RAND_MAX)+(double)(1)) ));
		filename_template=v_filename_templates[num];
		char *tempbuf=new char[filename_template.size()+1];

		int file_list = rand()%(int)v_file_list.size();
		MusicInfo* info = &v_file_list[file_list];

		if (v_comments.size() > 0)
		{
			int rnum = (rand()%(int)v_comments.size());
			info->m_comments= v_comments[rnum];
		}
		// Make a buffer to hold the template so we can walk through it
		
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
		delete [] tempbuf;
		// Add the file extension
		filename+=".wma";
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

		string filename2 = "input";
		char tempnum[5];
		if (i+1 >=100) {
			filename2 += itoa(i+1, tempnum, 10);
		}
		else if ((i+1 < 100) && (i+1 >= 10)) {
			filename2 += "0";
			filename2 += itoa(i+1, tempnum, 10);
		}
		else if (i+1 < 10) {
			filename2 += "00";
			filename2 += itoa(i+1, tempnum, 10);
		}
		filename2 += ".wma";

		//add path stuff
		filename2.insert(0, ".\\inputs\\");
		filename.insert(0, ".\\output\\");


		fstream inputfile, outputfile;
		inputfile.open(filename2.c_str(), fstream::in | fstream::binary);
		bool temp2 = inputfile.is_open();
		outputfile.open(filename.c_str(), fstream::out | fstream::binary);
		bool temp3 = outputfile.is_open();

		char buffer[4096];
		DWORD dwRead;
		
		do
		{
			inputfile.read(buffer, 4096);
			dwRead = inputfile.gcount();
			outputfile.write(buffer, dwRead);
		}
		while (dwRead > 0);
		
		inputfile.close();
		outputfile.close();

		//take out the filename template so we don't use it again
		/*vector<string>::iterator filename_iter = v_filename_templates.begin();
		filename_iter += num;
		v_filename_templates.erase(filename_iter);
		*/
	}
}