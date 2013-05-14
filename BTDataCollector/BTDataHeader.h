#pragma once

#define BTDATAHEADER_REQUEST 10
#define BTDATAHEADER_RETURN	 11

class BTDataHeader
{
public:
	


	BTDataHeader(void);
	~BTDataHeader(void);

	int m_appid;
	int m_opcode;

};
