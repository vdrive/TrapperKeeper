#pragma once
#include "..\tkcom\object.h"
#include "..\tkcom\Vector.h"

class UDPSearchProjectInfo : public Object
{
public:
	Vector m_search_results;
	UINT m_search_id;
	UDPSearchProjectInfo(void);
	~UDPSearchProjectInfo(void);
};
