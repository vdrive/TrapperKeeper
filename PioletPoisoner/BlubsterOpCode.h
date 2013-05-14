// BlubsterOpCode.h

#ifndef BLUBSTER_OP_CODE_H
#define BLUBSTER_OP_CODE_H

class BlubsterOpCode
{
public:
	enum op
	{
		Ping=0,
		Pong=1,
		Query=2,
		QueryHit=3,
		FileRequest=5
	};
};

#endif // BLUBSTER_OP_CODE_H