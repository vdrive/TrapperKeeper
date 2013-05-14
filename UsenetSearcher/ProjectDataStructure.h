// ProjectDataStructure.h

#ifndef PROJECT_DATA_STRUCTURE_H
#define PROJECT_DATA_STRUCTURE_H

#include "ProjectChecksum.h"

class ProjectDataStructure
{
public:
	virtual void Clear()=0;						// call this in the constructors of the derived classes
	virtual int GetBufferLength();
	virtual int WriteToBuffer(char *buf);
	virtual int ReadFromBuffer(char *buf)=0;	// returns buffer length

	virtual ProjectChecksum CalculateChecksum();
};

#endif // PROJECT_DATA_STRUCTURE_H