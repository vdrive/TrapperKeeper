#if !defined(AFX_MOVIEMAKERMODULE_H__DA4B627F_7986_444E_8B88_8604D1BF62AC__INCLUDED_)
#define AFX_MOVIEMAKERMODULE_H__DA4B627F_7986_444E_8B88_8604D1BF62AC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MovieMakerModule.h : header file
//

#include "RecordHeader.h"
#include "Record.h"
#include "MovieInfo.h"

/////////////////////////////////////////////////////////////////////////////
// MovieMakerModule window
class MediaManager;

class MovieMakerModule
{
// Construction
public:
	MovieMakerModule();
	~MovieMakerModule();

	void InitParent(MediaManager * parent);

	virtual void MakeEntry(MovieInfo info, int filesize, char * outputfolder);
	virtual int CopyMovie(char * moviename, int size, char * outputfolder);
	void CalculateHash(char * filename, char * hash);
	bool CopyDatabase(char * filename, int numfolders, char * dirname);
//	bool AddTag(CFile filename, int tag,int *recordlength,char * entry);
	bool ProcessTemplateFile(char * filepath, MovieInfo * info);
	void GenerateFilename(MovieInfo * info);
	void MakeDirectory( char * inputfolder, char * outputfolder);


private:
	bool ProcessTemplate(char * buf, MovieInfo * info);

	vector<string> v_descriptions;
	vector<string> v_tags;
	vector<string> v_types;
	vector<string> v_artists;

	MediaManager * p_parent;

};


#endif // !defined(AFX_MOVIEMAKERMODULE_H__DA4B627F_7986_444E_8B88_8604D1BF62AC__INCLUDED_)
