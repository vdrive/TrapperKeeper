//ProjectChecksums.h
#ifndef PROJECT_CHECKSUMS_H
#define PROJECT_CHECKSUMS_H

#include "ProjectChecksum.h"
#include "ProjectDataStructure.h"

class ProjectChecksums : public ProjectDataStructure
{
public:
	// Public Member Functions
	ProjectChecksums();
	~ProjectChecksums();

	void Clear();
	int GetBufferLength();
	int WriteToBuffer(char *buf);
	int ReadFromBuffer(char *buf);
	bool operator !=(ProjectChecksums &checksums);

	// Public Data Members
	vector<ProjectChecksum> v_checksums;
};

#endif  //PROJECT_CHECKSUMS_H