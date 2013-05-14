#ifndef _STL_UTILITIES_H
#define _STL_UTILITIES_H


#pragma warning(push, 2)
#include <algorithm>
#pragma warning(pop)


// Function to delete all elements which are assumed allocated with new
template <class T>
void DeleteAll(T& toDelete)
{
	for (T::iterator iter = toDelete.begin(); iter != toDelete.end(); ++iter)
	{
		delete *iter;
		*iter = NULL;
	}

	toDelete.clear();
}

// Function to delete all elements which are assumed allocated with new as an array
template <class T>
void DeleteAllArrays(T& toDelete)
{
	for (T::iterator iter = toDelete.begin(); iter != toDelete.end(); ++iter)
	{
		delete [] *iter;
		*iter = NULL;
	}
	toDelete.clear();
}

template <class T>
class CMyIterator
{
public:
	CMyIterator(const T& mem)
		: m_Base(mem)
	{
		m_Iter = m_Base.end(); // Force the user to call Begin()
	}

	void Begin()
	{
		m_Iter = m_Base.begin();
	}

	// Returns true as long is it can iterate
	operator bool()
	{
		return m_Iter != m_Base.end();
	}

	// Go to the next element (prefix version)
	CMyIterator<T>& operator++()
	{
		++m_Iter;
		return *this;
	}

	// Get the current item
	T::const_reference Get() const
	{
		return *m_Iter;
	}

	T::reference GetNonConst() const
	{
		return const_cast<T::reference>(*m_Iter);
	}
protected:
	T::const_iterator m_Iter; // Our iterator
	const T& m_Base; // The base class (needed for .begin(), and .end()
};

template <class T>
class CMyReverseIterator
{
public:
	CMyReverseIterator(const T& mem)
		: m_Base(mem)
	{
		m_Iter = m_Base.rend(); // Force the user to call Begin()
	}

	void Begin()
	{
		m_Iter = m_Base.rbegin();
	}

	// Returns true as long is it can iterate
	operator bool()
	{
		return m_Iter != m_Base.rend();
	}

	// Go to the next element (prefix version)
	CMyReverseIterator<T>& operator++()
	{
		++m_Iter;
		return *this;
	}

	// Get the current item
	T::const_reference Get() const
	{
		return *m_Iter;
	}

	T::reference GetNonConst() const
	{
		return const_cast<T::reference>(*m_Iter);
	}
protected:
	T::const_reverse_iterator m_Iter; // Our iterator
	const T& m_Base; // The base class (needed for .begin(), and .end()
};

#endif // _STL_UTILITIES_H