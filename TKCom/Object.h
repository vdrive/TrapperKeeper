#pragma once

//AUTHOR:  Ty Heath
//DATE MODIFIED:  1/13/2003
//CLASS:  Object
//Purpose:  Just a base class for all other classes so that they can be put into the nifty Vector class or etc.
//This class is a smart pointer class.  When all references to the object are gone it will delete itself.
//Unless the object is put into a Vector or other data structure class, it doesn't know if anything is referencing it, and thus it won't be a smart pointer
class Object
{
private:
	unsigned int m_reference_count;
protected:
	//UINT m_object_tag;
	//static UINT m_object_tag_counter;
public:
	friend class Vector;
	Object(void);
	virtual ~Object(void);

	// Compares this object to another.  Classes should override this function if they want sorting capability etc.
	virtual int CompareTo(Object* object)
	{
		return 0;
	}

	// Compares this object to another.  Classes should override this function if they want finding capability etc.
	virtual bool IsEqualTo(Object* object)
	{
		return false;
	}


	//adds a reference to this object.
	inline void ref(){
		m_reference_count++;
	}

	//adds a reference to this object.
	inline unsigned int GetRefCount(){
		return m_reference_count;
	}


	//removes a reference to this object, if nothing references it, it will cease to exist.
	inline void deref(){
		m_reference_count--;
		if(m_reference_count<1)
			delete this;
	}
};
