// CompressedQRPTable.h
#pragma once

class CompressedQRPTable
{
public:
	CompressedQRPTable();
	~CompressedQRPTable();
	CompressedQRPTable(CompressedQRPTable &table);	// copy constructor
	
	void Clear();

	void SetData(char *data,unsigned int data_len);
	char *GetData();
	unsigned int GetLen();

	CompressedQRPTable& operator=(CompressedQRPTable &table);	// assignment operator

private:
	char *p_data;
	unsigned int m_data_len;
};