#include "StdAfx.h"
#include "vector.h"

Vector::Vector()
{
	size=0;
	tlength=0;
	vector=NULL;
	//b_deleteObjectsWhenDone=_b_deleteObjectsWhenDone;
}

Vector::~Vector(void)
{
	Clear();
}

//this function grows the vector
void Vector::EnsureSize(int _size)
{
	if(tlength>=_size)
		return;

	Object **tv=new Object*[_size];	//make a new bigger array
	for(int i=0;i<size;i++){		//do a simple array copy
		tv[i]=vector[i];
	}
	delete []vector;	//free the array of pointers that was too small because we copied them to a bigger array
	tlength=_size;
	vector=tv;			
}

int Vector::Add(Object* element,int b_allowDuplicates)
{
	if(!b_allowDuplicates){
		if(Get(element)!=-1)
			return size;
	}

	element->ref();  //increment this objects reference count

	if(tlength<size+1)
		EnsureSize((size+1)<<1);  //double the size of the array.  this will make allocations rare to help reduce memory fragmentation, copying, etc.

	vector[size]=element;
	size++;

	return size;//return the new size of the vector
}

// Returns the 0 based indexed element of the array
Object* Vector::Get(int index)
{
	if(index>=0 && index<size){
		return vector[index];
	}
	return NULL;
}

void Vector::Clear()
{
	for(int i=0;i<size;i++){
		vector[i]->deref();
	}

	delete [] vector;
	size=NULL;
	tlength=NULL;
	vector=NULL;
}

//Call this function to append another vector onto the end of this one
int Vector::Append(Vector* _vector,int b_allowDuplicates)
{
	for(int i=0;i<_vector->Size();i++){
		Add(_vector->Get(i),b_allowDuplicates);
	}
	return size;
}

// returns the size of the vector, or how many objects it has in it etc
int Vector::Size(void)
{
	return size;
}

// Makes this vector an intersection of itself and the Vector* parameter.  (An intersection of 2 sets is a new set that contains only elements that were in both sets)
// This vector will be either the same size or smaller
int Vector::Intersect(Vector* _vector)
{
	if(_vector==NULL)
		return size;
	for(int i=0;i<size;i++){
		Object *obj=vector[i];
		if(_vector->Get(obj)==-1){  //if the other vector doesn't have this object then we need to delete it from this vector
			this->Remove(obj);
			i--;	//don't skip anything
		}
	}
	return size;
}

// returns the index of the first reference to 'target'.  or -1 if no such reference is found
int Vector::Get(Object* target,int offset)
{
	if(offset<0)
		offset=0;
	for(int i=offset;i<size;i++){
		if(vector[i]==target)
			return i;
	}
	return -1;
}

// returns number of times this object was found and removed
int Vector::Remove(Object* target)
{
	int found=0;
	for(int i=0;i<size;i++){
		if(vector[i]==target){
			vector[i]->deref();
			for(int j=i;j<size-1;j++){  //shift the rest of the array back by 1
				vector[j]=vector[j+1];
			}
			size--;  //our size just shrank by 1
			i--;     //lets not skip 1
			found++;
		}	
	}

	return found;
}

// Removes an object from the specified 0 based index of the vector.  Returns the number of objects removed
int Vector::Remove(int index)
{
	if(index<0 || index>=size)
		return 0;

	vector[index]->deref();
	for(int j=index;j<size-1;j++){  //shift the rest of the array back by 1
		vector[j]=vector[j+1];
	}	

	return 1;

	/*
	if(b_deleteObject)		//delete the object if necessary
		delete vector[index];

	for(int j=index;j<size-1;j++){  //shift the rest of the array back by 1
		vector[j]=vector[j+1];
	}
	size--;  //our size just shrank by 1
	*/
	//return 1;
}

// Removes a range of elements from the vector. 
// Make sure you have all duplicates removed from this vector or results may be unexpected
int Vector::RemoveRange(int start, int count)
{
	for(int i=0;i<count;i++){
		Remove(start);
	}
	return 0;
}

// Creates a union of this vector and another.  The result is contained in this vector.  (A union is a new set that contains all elements of both argument sets)
int Vector::Union(Vector* _vector)
{
	int len=_vector->Size();
	for(int i=0;i<len;i++){
		Object* obj=_vector->Get(i);
		if(this->Get(obj)==-1)
			this->Add(obj);
	}
	return size;
}

// subtracts all the elements in one vector from this vector.  The result is stored in this vector
int Vector::Subtract(Vector* _vector)
{
	int len=_vector->Size();
	for(int i=0;i<len;i++){
		this->Remove(_vector->Get(i));
	}
	return size;
}

// function used by the Sort function to sort in ascending order
int compareAscending( const void *arg1, const void *arg2 )
{
	/* Let the objects compare themselves through their overriden CompareTo method: */
	return ((Object*)arg1)->CompareTo((Object*)arg2);
}

// function used by the Sort function to sort in descending order
int compareDescending( const void *arg1, const void *arg2 )
{
	/* Let the objects compare themselves through their overriden CompareTo method: */
	return ((Object*)arg2)->CompareTo((Object*)arg1);
}

// uses a quicksort algorithm to sort the objects.  Objects must have the CompareTo function overridden to be sorted properly
void Vector::Sort(int b_ascending)
{
	if(b_ascending)
		qsort( (void *)vector, (size_t)size, sizeof( Object * ), compareAscending );
	else
		qsort( (void *)vector, (size_t)size, sizeof( Object * ), compareDescending );

}

// Makes the vector so that each item within it is unique
// A Sweet n^3 algorithm.
// If you need a better algorithm make one.  An nlog(n) algorithm consists of sorting the vector using an nlog(n) comparitive alg, then going through it and adding non dupes to a new vector.  The current algorithm will be extremely slow if N is large and num dupes is large, but is here for rare non critical functionality.
int Vector::RemoveDuplicates(void)
{
	//this vector should be sorted first to get an nlog(n) algorithm.  But the user may not want their damn vector sorted.  So this algorithm is and will always be fucked
	for(int i=0;i<size;i++){
		for(int j=i+1;j<size;j++){
			if(vector[j]==vector[i]){
				Remove(j);	
				j--;
			}
		}
	}
	return 0;
}

// Copies another vector into this vector.  It deletes the current objects in this vector if that flag is set
void Vector::Copy(Vector* _target)
{
	Clear();
	if(_target==NULL)
		return;

	int len=_target->Size();

	Object **array=new Object*[len];
	for(int i=0;i<len;i++){
		array[i]=_target->vector[i];
		array[i]->ref();  //make sure the object knows that this vector now references it as well
	}
	size=len;
	tlength=size;
}

