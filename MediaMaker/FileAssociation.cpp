// FileAssociation.cpp

#include "stdafx.h"
#include "FileAssociation.h"

//
//
//
FileAssociation::FileAssociation()
{
	
	memset(m_path,0,sizeof(m_path));
	memset(m_assoc_path,0,sizeof(m_assoc_path));
}

//
//
//
bool FileAssociation::Save()
{
	CFile file;
	//Open the maps file and write the file association
	if(file.Open("maps.dat",CFile::modeWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::typeBinary|CFile::shareDenyWrite)==FALSE)
	{
		return false;
	}

	file.SeekToEnd();
	file.Write(this,sizeof(FileAssociation));
	file.Close();
	return true;
}