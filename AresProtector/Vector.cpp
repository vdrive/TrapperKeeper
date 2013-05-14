#include "StdAfx.h"
#include "Vector.h"

Vector::Vector()
{
	size=0;
	tlength=0;
	vector_objects=NULL;
	//b_deleteObjectsWhenDone=_b_deleteObjectsWhenDone;
}

Vector::~Vector(void)
{
	Clear();
}

//this function grows the vector_objects
void Vector::EnsureSize(UINT _size)
{
	CSingleLock sl(&m_lock,TRUE);

	Object **tv=new Object*[_size];	//make a new bigger array
	UINT limit=min(size,_size);
	for(UINT i=0;i<limit;i++){		//do a simple array copy
		tv[i]=vector_objects[i]; //reassign the pointers
	}
	delete []vector_objects;	//free the array of pointers that was too small because we copied them to a bigger array
	tlength=_size;		//set the capacity of the vector_objects to the new size
	vector_objects=tv;			//
}

UINT Vector::Add(Object* element,int b_allowDuplicates)
{
	return AddEx(element,b_allowDuplicates,true);
}

//A non threadsafe but much faster version of add
UINT Vector::AddEx(Object* element,int b_allowDuplicates,bool b_thread_safe)
{
	CSingleLock sq(&m_lock);
	if(b_thread_safe){
		sq.Lock();
	}
	if(element==NULL)
		return size;
	if(!b_allowDuplicates){
		if(GetIndex(element)!=-1)
			return size;
	}

	element->ref();  //increment this objects reference count

	if(tlength<size+1)
		EnsureSize((size+1)<<1);  //double the size of the array.  this will make allocations rare to help reduce memory fragmentation, copying, etc.

	vector_objects[size]=element;
	size++;

	return size;//return the new size of the vector_objects
}


UINT Vector::Insert(Object* element,UINT index)
{
	CSingleLock sl(&m_lock,TRUE);

	element->ref();  //increment this objects reference count

	if(tlength<size+1)
		EnsureSize((size+1)<<1);  //double the size of the array.  this will make allocations rare to help reduce memory fragmentation, copying, etc.

	if(index>size)
		index=size;

	for(UINT i=size;i>index;i--){
		vector_objects[i]=vector_objects[i-1];
	}
	//if(size-index>0)
	//	memmove(vector_objects+index+1,vector_objects+index,size-index);  //slide the array forward by 1

	vector_objects[index]=element;
	size++;

	return size;//return the new size of the vector_objects
}

// Returns the 0 based indexed element of the array
Object* Vector::Get(UINT index)
{
	CSingleLock sl(&m_lock,TRUE);
	if(index>=0 && index<size){
		return vector_objects[index];
	}
	return NULL;
}

void Vector::Clear()
{
	CSingleLock sl(&m_lock,TRUE);
	for(UINT i=0;i<size;i++){
		vector_objects[i]->deref();
	}

	if(vector_objects!=NULL){
		delete [] vector_objects;
	}
	size=0;
	tlength=0;
	vector_objects=NULL;
}

//Call this function to append another vector_objects onto the end of this one
int Vector::Append(Vector* _vector,int b_allowDuplicates)
{
	CSingleLock sl(&m_lock,TRUE);
	for(UINT i=0;i<_vector->Size();i++){
		AddEx(_vector->Get(i),b_allowDuplicates,false);
	}
	return size;
}




// Makes this vector_objects an intersection of itself and the Vector* parameter.  (An intersection of 2 sets is a new set that contains only elements that were in both sets)
// This vector_objects will be either the same size or smaller
int Vector::Intersect(Vector* _vector)
{
	CSingleLock sl(&m_lock,TRUE);
	if(_vector==NULL)
		return size;
	for(UINT i=0;i<size;i++){
		Object *obj=vector_objects[i];
		if(_vector->GetIndex(obj)==-1){  //if the other vector_objects doesn't have this object then we need to delete it from this vector
			this->RemoveByReference(obj);
			i--;	//don't skip anything
		}
	}
	return size;
}

// returns the index of the first reference to 'target'.  or -1 if no such reference is found
int Vector::GetIndex(Object* target,UINT offset)
{
	CSingleLock sl(&m_lock,TRUE);
	if(offset<0)
		offset=0;
	for(UINT i=offset;i<size;i++){
		if(vector_objects[i]==target)
			return i;
	}
	return -1;
}

// returns number of times this object was found and removed
int Vector::RemoveByReference(Object* target)
{
	CSingleLock sl(&m_lock,TRUE);
	int found=0;
	for(UINT i=0;i<size;i++){
		if(vector_objects[i]==target){
			vector_objects[i]->deref();
			for(UINT j=i;j<size-1;j++){  //shift the rest of the array back by 1
				vector_objects[j]=vector_objects[j+1];
			}
			size--;  //our size just shrank by 1
			i--;     //lets not skip 1
			found++;
		}	
	}

	return found;
}

// Removes an object from the specified 0 based index of the vector_objects.  Returns the number of objects removed
int Vector::Remove(UINT index)
{
	return RemoveEx(index,true);
}

// Removes an object from the specified 0 based index of the vector_objects.  Returns the number of objects removed
int Vector::RemoveEx(UINT index,bool b_thread_safe)
{
	CSingleLock sq(&m_lock);
	if(b_thread_safe){
		sq.Lock();
	}
	if(index<0 || index>=size)
		return 0;

	vector_objects[index]->deref(); //let the object delete itself if there are no more references to it.
	for(UINT j=index;j<size-1;j++){  //shift the rest of the array back by 1
		vector_objects[j]=vector_objects[j+1];
	}	

	size--;

	return 1;
}

// Removes a range of elements from the vector_objects.  More efficient than calling remove for each one.
void Vector::RemoveRange(UINT start, UINT count)
{
	CSingleLock sl(&m_lock,TRUE);
	if(start>size || count<1)
		return;
	count=min(count,size-start);
	UINT index=0;
	for(UINT i=start;i<size;i++){
		if(i-start<count)
			vector_objects[i]->deref();
		if(i+count<size){
			vector_objects[i]=vector_objects[i+count];
		}
	}

	//memmove(vector_objects+start,vector_objects+start+count,sizeof(UINT)*(size-(start+count))); //move end of vector_objects back

	size-=count;

	return;
}

// Creates a union of this vector_objects and another.  The result is contained in this vector_objects.  (A union is a new set that contains all elements of both argument sets)
int Vector::Union(Vector* _vector)
{
	CSingleLock sl(&m_lock,TRUE);
	int len=_vector->Size();
	for(int i=0;i<len;i++){
		Object* obj=_vector->Get(i);
		if(this->GetIndex(obj)==-1)
			this->AddEx(obj,true,false);
	}
	return size;
}

// subtracts all the elements in one vector_objects from this vector_objects.  The result is stored in this vector_objects
int Vector::Subtract(Vector* _vector)
{
	CSingleLock sl(&m_lock,TRUE);
	int len=_vector->Size();
	for(int i=0;i<len;i++){
		this->RemoveByReference(_vector->Get(i));
	}
	return size;
}

// function used by the Sort function to sort in ascending order
int compareAscending( const void *arg1, const void *arg2 )
{
	/* Let the objects compare themselves through their overriden CompareTo method: */
	return (*(Object**)arg1)->CompareTo(*(Object**)arg2);
}

// function used by the Sort function to sort in descending order
int compareDescending( const void *arg1, const void *arg2 )
{
	/* Let the objects compare themselves through their overriden CompareTo method: */
	return (*(Object**)arg2)->CompareTo(*(Object**)arg1);
}

// uses a quicksort algorithm to sort the objects.  Objects must have the CompareTo function overridden to be sorted properly
void Vector::Sort(int b_ascending)
{
	CSingleLock sl(&m_lock,TRUE);
	if(b_ascending)
		qsort( (void *)vector_objects, (size_t)size, sizeof( Object * ), compareAscending );
	else
		qsort( (void *)vector_objects, (size_t)size, sizeof( Object * ), compareDescending );

}

// Makes the vector_objects so that each item within it is unique
// A Sweet n^3 algorithm.
// If you need a better algorithm make one.  An nlog(n) algorithm consists of sorting the vector_objects using an nlog(n) comparitive alg, then going through it and adding non dupes to a new vector_objects.  The current algorithm will be extremely slow if N is large and num dupes is large, but is here for rare non critical functionality.
int Vector::RemoveDuplicates(void)
{
	CSingleLock sl(&m_lock,TRUE);
	//this vector_objects should be sorted first to get an nlog(n) algorithm.  But the user may not want their damn vector_objects sorted.  So this algorithm is and will always be fucked
	for(UINT i=0;i<size;i++){
		for(UINT j=i+1;j<size;j++){
			if(vector_objects[j]==vector_objects[i]){
				Remove(j);	
				j--;
			}
		}
	}
	return 0;
}

// Copies another vector_objects into this vector_objects.  It deletes the current objects in this vector_objects if that flag is set
void Vector::Copy(Vector* _target)
{
	CSingleLock sl(&m_lock,TRUE);
	Clear();
	if(_target==NULL)
		return;

	int len=_target->Size();

	Object **array=new Object*[len];
	for(int i=0;i<len;i++){
		array[i]=_target->vector_objects[i];
		array[i]->ref();  //make sure the object knows that this vector_objects now references it as well
	}
	size=len;
	tlength=size;
	vector_objects=array;
}


void Vector::Optimize(void){
	CSingleLock sl(&m_lock,TRUE);

	tlength=size+10;  //set our capacity to only a tiny bit bigger than it needs to be.  

	Object **tv=new Object*[tlength];	//make a new array that is more appropriately sized
	for(UINT i=0;i<size;i++){		//do a simple array copy
		tv[i]=vector_objects[i]; //reassign the pointers
	}
	delete []vector_objects;	//free the array of pointers that was possibly much bigger than it needed to be.
	vector_objects=tv;			//
}

int Vector::BinaryFind(Object* target){

	CSingleLock sl(&m_lock,TRUE);
	if(size==0)
		return -1;
	int low=0;
	int high=max(0,size-1);
	int middle=0;
	//do a binary search to determine where we should insert this file into the array
	for(;low<=high;){
		middle=((high+low)>>1);
		Object* fo=(Object*)vector_objects[middle];
		int result=target->CompareTo(fo);
		if(result==0){
			return middle;
		}
		else if( result<0 )
			high = middle - 1;		//search low end of array
		else
			low = middle + 1;		//search high end of array

	}

	return -1;

	
	/*
	Object **ref=(Object**) bsearch( (void *)target, (void *)vector_objects , (size_t)size , sizeof( Object * ), compareAscending);
	if(ref==NULL)
		return -1;
	else return (int)(ref-vector_objects);  //return offset of object in array
	*/
}
