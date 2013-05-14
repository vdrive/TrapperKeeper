#pragma once
#include "object.h"

class ComSubscriber :
	public Object
{
public:
	UINT m_op_code;
	UINT m_from_app;
	UINT m_ref_count;

	ComSubscriber(UINT op_code,UINT from_app);
	~ComSubscriber(void);
	void AddRef(void);
	void DeRef(void);
	bool IsEmpty(void);
};
