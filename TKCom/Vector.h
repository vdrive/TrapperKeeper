#pragma once
#include "object.h"
#include <afxmt.h>
//AUTHOR:  Ty Heath
//DATE MODIFIED:  1/13/2003
//CLASS: Vector
//PURPOSE:  This is just a basic container class that encapsulates a vector data structure.  All memory management is taken care of.
//NOTES:
//1. ALL OBJECTS PLACED INTO THIS DATASTRUCTURE MUST BE CREATED ON THE HEAP VIA A 'new' OPERATOR
//2. When an object has been removed from all Vector and/or other data storage utilities (built by Ty Heath) it will be deleted.
//3. A Clean fast object oriented pointer type vector class.  Any object derived from Object can be stored in it.
//4. This class is threadsafe.  An external guard needs to be made if you plan on removing objects.  An object could be deleted while you are using its pointer from a Get().
class Vector :	public Object
{
protected:
	//attributes internally used to manage the data structure.
	UINT size;
	UINT tlength;
	Object **vector_objects;
	
	//Thread safe access to the data
	CCriticalSection m_lock;

public:
	Vector();
	virtual ~Vector(void);
protected:										//A function for internal memory management.
public:

	//call make a vector's capacity equal to the parameter _size.  It will not truncate the vector
	void EnsureSize(UINT _size);

	//Add an object.  This class is designed to work with objects on the heap  It will automatically resize the array as necessary, optimized for as few allocations as possible while adding sets of data
	UINT Add(Object* element,int b_allowDuplicates=1);	

	//Add an object.  This class is designed to work with objects on the heap  It will automatically resize the array as necessary, optimized for as few allocations as possible while adding sets of data.
	//this extended version of add allows you to disable the thread safeness for faster operations
	UINT AddEx(Object* element,int b_allowDuplicates,bool b_thread_safe);

	

	//Inserts an object.  This class is designed to work with objects on the heap  It will automatically resize the array as necessary, optimized for as few allocations as possible while adding sets of data
	UINT Insert(Object* element,UINT index);
	
	// Returns the 0 based indexed element of the vector
	Object* Get(UINT index);											

	//removes all elements from the vector, deleting them if desired.  If they are not deleted, then you must delete those objects somewhere sometime if they exist on the heap!
	void Clear();							

	// appends _vector onto this vector.  The new array may contain duplicates
	int Append(Vector* _vector,int b_allowDuplicates=0);

	// returns the size of the vector, or how many objects it has in it etc
	inline UINT Size(void){return size;}  //this will be called a lot so it is inline

	// Makes this vector an intersection of itself and the Vector* parameter.  (An intersection of 2 sets is a new set that contains only elements that were in both sets)
	int Intersect(Vector* _vector);

	// returns the index of the first reference to 'target'.  or -1 if no such reference is found
	int GetIndex(Object* target,UINT offset = 0);

	// returns 1 if the object was found, and 0 otherwise
	int RemoveByReference(Object* target);

	// Creates a union of this vector and another.  The result is contained in this vector.  (A union is a new set that contains all elements of both argument sets)
	int Union(Vector* _vector);

	// subtracts all the elements in one vector from this vector.  The result is stored in this vector
	int Subtract(Vector* _vector);

	// uses a quicksort algorithm to sort the objects.  Objects must have the CompareTo function overridden to be sorted properly
	void Sort(int b_ascending);

	// Removes a range of elements from the vector
	void RemoveRange(UINT start, UINT count);

	// Removes an object from the specified 0 based index of the vector
	int Remove(UINT index);

	// Removes an object from the specified 0 based index of the vector.  This version allows the disabling of threadsafeness for a faster operation
	int RemoveEx(UINT index,bool b_thread_safe);

	// Makes the vector so that each item within it is unique
	int RemoveDuplicates(void);

	// Copies another vector into this vector
	void Copy(Vector* _target);

	// Call to make the internal size of the vector no bigger than it needs to be
	void Optimize(void);

	int BinaryFind(Object *target);
};
