#pragma once
#include "..\tkcom\Object.h"
#include "..\tkcom\Vector.h"
#include "hashobject.h"

class HashTable
{
private:
	UINT m_number_elements;
	UINT m_table_size;
	bool mb_sorted;
	void GrowTable();
public:
	HashTable(int table_size,bool b_sorted);
	~HashTable(void);
	Vector *mp_vector_array;  //array of X vectors
	void Add(HashObject* object);
	Vector* GetList(UINT hash_ref);
	void EnumerateTable(Vector& v_items);
	UINT GetTableSize(void);
	void RemoveObjectByObjectCompare(HashObject* target);
	void RemoveObjectByHashRef(UINT hash_key);
	void Clear(void);
	void SortTable(void);
	HashObject* GetObjectByHashRef(UINT hash_key);
	
};
