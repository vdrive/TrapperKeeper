#pragma once
#include "object.h"
//AUTHOR:  Ty Heath
//DATE MODIFIED:  1/13/2003
//CLASS: Vector
//PURPOSE:  This is just a basic container class that encapsulates a vector data structure.  All memory management is taken care of.
//NOTES:
//ALL OBJECTS PLACED INTO THIS DATASTRUCTURE MUST BE CREATED ON THE HEAP
//A Clean fast object oriented vector class.  Faster than the STL vector class.  Any object derived from Object can be stored in it.
class Vector :
	public Object
{
protected:
	int size;
	int tlength;
	Object **vector;
	//int b_deleteObjectsWhenDone;

public:
	Vector();
	virtual ~Vector(void);
protected:
	void EnsureSize(int _size);										//A function for internal memory management.
public:
	int Add(Object* element,int b_allowDuplicates=0);										//Add an object.  This class is designed to work with objects on the heap, however objects on the stack will work as well, just make sure they aren't popped while this class is using them.  It will automatically resize the array as necessary, optimized for as few allocations as possible while adding sets of data
	Object* Get(int index);											// Returns the 0 based indexed element of the vector
	void Clear();							//removes all elements from the vector, deleting them if desired.  If they are not deleted, then you must delete those objects somewhere sometime if they exist on the heap!

	// appends _vector onto this vector.  The new array may contain duplicates
	int Append(Vector* _vector,int b_allowDuplicates=0);
	// returns the size of the vector, or how many objects it has in it etc
	int Size(void);
	// Makes this vector an intersection of itself and the Vector* parameter.  (An intersection of 2 sets is a new set that contains only elements that were in both sets)
	int Intersect(Vector* _vector);
	// returns the index of the first reference to 'target'.  or -1 if no such reference is found
	int Get(Object* target,int offset = 0);
	// returns 1 if the object was found, and 0 otherwise
	int Remove(Object* target);
	// Creates a union of this vector and another.  The result is contained in this vector.  (A union is a new set that contains all elements of both argument sets)
	int Union(Vector* _vector);
	// subtracts all the elements in one vector from this vector.  The result is stored in this vector
	int Subtract(Vector* _vector);
	// uses a quicksort algorithm to sort the objects.  Objects must have the CompareTo function overridden to be sorted properly
	void Sort(int b_ascending);
	// Removes a range of elements from the vector
	int RemoveRange(int start, int count);
	// Removes an object from the specified 0 based index of the vector
	int Remove(int index);
	// Makes the vector so that each item within it is unique
	int RemoveDuplicates(void);
	// Copies another vector into this vector
	void Copy(Vector* _target);
};
