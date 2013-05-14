// DatabaseItem.h

#pragma once

#include <vector>
using namespace std;

class DatabaseItem
{
public:
	DatabaseItem();
	~DatabaseItem();
	DatabaseItem(const DatabaseItem &item);				// copy constructor
	DatabaseItem& operator=(const DatabaseItem &item);	// assignment operator
//	bool operator<(const DatabaseItem &item) const;			// less than operator

	// Reading Data
	unsigned int ID();
	char *Column(unsigned int col);

	// Writing Data
	void ID(unsigned int id);
	void AddColumn(char *buf);

private:
	unsigned int m_id;
	vector<char *> v_columns;
};