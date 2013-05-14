#pragma once
#include "..\tkcom\object.h"
#include "..\tkcom\vector.h"

class FileBufferIndex : public Object
{
	Vector mv_files;
public:
	FileBufferIndex(void);
	~FileBufferIndex(void);
};
