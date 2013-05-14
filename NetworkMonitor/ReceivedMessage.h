#pragma once
#include "..\TKCom\Object.h"
#include "..\TKCom\Buffer2000.h"

class ReceivedMessage : public Object
{
private:
	string m_source_ip;
	Buffer2000 m_msg;
	UINT m_header;
public:
	ReceivedMessage(const char* source_ip,byte* data,UINT data_length);
	~ReceivedMessage(void);

	UINT GetHeader(void);
	Buffer2000* GetData();
	const char* GetSource(void);
};
