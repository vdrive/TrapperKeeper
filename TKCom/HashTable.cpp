#include "StdAfx.h"
#include "hashtable.h"

HashTable::HashTable(int table_size,bool b_sorted)
{
	mb_sorted=b_sorted;
	table_size=max(table_size,1);
	m_table_size=table_size;
	mp_vector_array=new Vector[table_size];
	m_number_elements=0;
}

HashTable::~HashTable(void)
{
	delete [] mp_vector_array;
}

void HashTable::Add(HashObject* object)
{
	int hash_ref=object->GetHashRef();
	Vector *v=&mp_vector_array[hash_ref%m_table_size];

	m_number_elements++;

	v->Add(object);
	if(mb_sorted){
		v->Sort(1);
	}

	//our load factor will be 1.0, if the number of elements in the table is greater than the table capacity, we increase that capacity.
	if(m_number_elements>m_table_size){
		GrowTable();  
	}
}

Vector* HashTable::GetList(UINT hash_ref)
{
	return &mp_vector_array[hash_ref%m_table_size];
}

void HashTable::EnumerateTable(Vector& v_items)
{
	for(int i=0;i<(int)m_table_size;i++){
		v_items.Append(&mp_vector_array[i]);
	}
}

UINT HashTable::GetTableSize(void)
{
	return m_table_size;
}

void HashTable::RemoveObjectByObjectCompare(HashObject* target)
{
	Vector *v=&mp_vector_array[target->GetHashRef()%m_table_size];

	if(mb_sorted){
		int loc=v->BinaryFind(target);
		if(loc!=-1){
			v->Remove(loc);
			m_number_elements--;
		}
	}
	else{
		int len=v->Size();
		for(int i=0;i<len;i++){
			if(v->Get(i)->CompareTo(target)==0){
				v->Remove(i);
				m_number_elements--;
				return;
			}
		}
	}
}

void HashTable::RemoveObjectByHashRef(UINT hash_key)
{
	Vector *v=&mp_vector_array[hash_key%m_table_size];

	int len=v->Size();
	for(int i=0;i<len;i++){
		if(((HashObject*)v->Get(i))->GetHashRef()==hash_key){
			v->Remove(i);
			m_number_elements--;
			return;
		}
	}
}

void HashTable::Clear(void)
{
	for(int i=0;i<(int)m_table_size;i++){
		mp_vector_array[i].Clear();
	}
	m_number_elements=0;
}

//this could be a very expensive operation
void HashTable::SortTable(void)
{
	for(int i=0;i<(int)m_table_size;i++){
		mp_vector_array[i].Sort(1);
	}
}

//returns the first object that matches the key
HashObject* HashTable::GetObjectByHashRef(UINT hash_key)
{
	Vector *v=&mp_vector_array[hash_key%m_table_size];

	int len=v->Size();
	for(int i=0;i<len;i++){
		if(((HashObject*)v->Get(i))->GetHashRef()==hash_key){
			return (HashObject*)v->Get(i);
		}
	}
	return NULL;
}

void HashTable::GrowTable(){  //double the capacity
	Vector v_items;
	EnumerateTable(v_items);

	delete []mp_vector_array;
	m_number_elements=0;

	m_table_size<<=1;  //left shift capacity by 1

	mp_vector_array=new Vector[m_table_size];	

	int len=v_items.Size();

	bool b_sort=mb_sorted;  //remember whether or not it was sorted
	mb_sorted=false;  //turn sorting off to make the adds much faster

	for(int i=0;i<len;i++){
		Add((HashObject*)v_items.Get(i));
	}

	if(b_sort){
		SortTable();
	}
}
