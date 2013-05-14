// Supernode.h

#ifndef SUPERNODE_H
#define SUPERNODE_H

class Supernode
{
public:
	Supernode();
	void Clear();

	unsigned int m_ip;
	unsigned short int m_port;
	unsigned char m_load_percentage;	// 0-99 ... lower is better
	unsigned char m_availability;		// 0=candidate, 1=being probed, 2=dead/unreachable ? ... 0 is desired
	unsigned int m_creation_time;		// more recent is desired
};

#endif // SUPERNODE_H