// QRPKeywords.h

#ifndef QRP_KEYWORDS_H
#define QRP_KEYWORDS_H

#include "ProjectDataStructure.h"

class QRPKeywords : public ProjectDataStructure
{
public:
	
	// Public Member Functions
	QRPKeywords();
	~QRPKeywords();

	void Clear();
	int GetBufferLength();
	int WriteToBuffer(char *buf);
	int ReadFromBuffer(char *buf);

	// Public Data Members
	vector<string> v_keywords;
};

#endif // QRP_KEYWORDS_H