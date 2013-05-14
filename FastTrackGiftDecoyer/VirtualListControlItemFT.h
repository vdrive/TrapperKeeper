// VirtualListControlItemFT.h

#ifndef VIRTUAL_LIST_CONTROL_ITEM_H
#define VIRTUAL_LIST_CONTROL_ITEM_H

class VirtualListControlItemFT
{
public:
	VirtualListControlItemFT();
	bool operator<(VirtualListControlItemFT &item);

	unsigned int m_mod;
	unsigned int m_sock;

	unsigned int m_remote_ip;
	string m_filename;

	unsigned int m_start;
	unsigned int m_end;
	unsigned int m_total_size;

	unsigned int m_num_sent;
	CTimeSpan m_time_elapsed;

	string m_client;
};

#endif // VIRTUAL_LIST_CONTROL_ITEM_H