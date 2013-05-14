// FileAssociation.h

#ifndef FILE_ASSOCIATION_H
#define FILE_ASSOCIATION_H

class FileAssociation
{
public:
	FileAssociation();
	bool Save();

	char m_path[256+1];
	char m_assoc_path[256+1];
};

#endif // FILE_ASSOCIATION_H