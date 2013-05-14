#pragma once

//AUTHOR:  Ty Heath
//DATE MODIFIED:  1/13/2003
//CLASS:  Object
//Purpose:  Just a base class for all other classes so that they can be put into the nifty Vector class or etc.
//This class is a smart pointer class.  When all references to the object are gone it will delete itself.
//Unless the object is put into a Vector or other data structure class, it doesn't know if anything is referencing it, and thus it won't be a smart pointer
class Object
{
protected:
	unsigned int referenceCount;

public:
	friend class Vector;
	Object(void);
	virtual ~Object(void);

	// Compares this object to another.  Classes should override this function if they want sorting capability etc.
	virtual int CompareTo(Object* object)
	{
		return 0;
	}

	//adds a reference to this object.  Keeping something from 
	inline void ref(){
		referenceCount++;
	}

	inline void deref(){
		referenceCount--;
		if(referenceCount<1)
			delete this;
	}
};
