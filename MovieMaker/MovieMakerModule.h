#if !defined(AFX_MOVIEMAKERMODULE_H__DA4B627F_7986_444E_8B88_8604D1BF62AC__INCLUDED_)
#define AFX_MOVIEMAKERMODULE_H__DA4B627F_7986_444E_8B88_8604D1BF62AC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MovieMakerModule.h : header file
//

#include "RecordHeader.h"
#include "Record.h"
#include "../SamplePlugIn/dll.h"
#include "../DllLoader/AppID.h"
#include "MovieMakerDlg.h"
#include "MovieInfo.h"

/////////////////////////////////////////////////////////////////////////////
// MovieMakerModule window

class MovieMakerModule :	public Dll
{
// Construction
public:
	MovieMakerModule();
	~MovieMakerModule();

	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();

	void MakeEntry(MovieInfo info, int filesize);
	int CopyMovie(char * moviename, int size);
	void ReadEntry();
	void CalculateHash(char * filename, char * hash);
	bool CopyDatabase(char * filename, int numfolders);
	bool AddTag(CFile filename, int tag,int *recordlength,char * entry);
	bool ProcessTemplateFile(char * filepath, MovieInfo * info);
	void GenerateFilename(MovieInfo * info);

private:
	bool ProcessTemplate(char * buf, MovieInfo * info);

	vector<string> v_descriptions;
	vector<string> v_tags;
	vector<string> v_types;

	CMovieMakerDlg m_dlg;
};


#endif // !defined(AFX_MOVIEMAKERMODULE_H__DA4B627F_7986_444E_8B88_8604D1BF62AC__INCLUDED_)
