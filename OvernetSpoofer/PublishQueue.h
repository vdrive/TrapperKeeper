#pragma once
#include "MetaData.h"

class PublishQueue
{
public:
	PublishQueue(void);
	~PublishQueue(void);
	void Clear();
	bool operator < (const PublishQueue&  other)const;
	bool operator ==(PublishQueue &other);


	UINT m_ip;
	short unsigned int m_port;
	vector<MetaData> v_to_be_published;

	MetaData PublishAckReceived();
	MetaData& ReturnCurrentMetaData();

};
class PublishQueueHash : public hash_compare <PublishQueue>
{
public:
	using hash_compare<PublishQueue>::operator();
	size_t operator()(const PublishQueue& key)const
	{
		return key.m_ip;
	}
};