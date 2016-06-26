#include "df_common.h"

#include <stdlib.h>


template <class T>
DArray<T>::DArray()
:	m_arraySize(0),
m_nextIndex(0),
m_array(NULL)
{
}


template <class T>
DArray<T>::DArray(DArray <T> const &other)
{
    m_arraySize = other.m_arraySize;
    m_nextIndex = other.m_nextIndex;
    if (m_arraySize > 0)
    {
        m_array = new T[m_arraySize];
        for (int i = 0; i < m_arraySize; i++)
        {
            m_array[i] = other.m_array[i];
        }
    }
    else
    {
        m_array = NULL;
    }
}


template <class T>
DArray<T>::~DArray()
{
    Empty();
}


template <class T>
inline void DArray<T>::SetSize(unsigned int newsize)
{
	if (newsize > m_arraySize) 
	{
		unsigned int oldarraysize = m_arraySize;

		m_arraySize = newsize;
		T *temparray = new T[ m_arraySize ];
		
		for (unsigned int a = 0; a < oldarraysize; ++a) 
		{
			temparray[a] = m_array[a];
		}
		
		delete [] m_array;
		m_array = temparray;
	}
	else if (newsize < m_arraySize) 
	{
		m_arraySize = newsize;
		T *temparray = new T[m_arraySize];

		for (unsigned int a = 0; a < m_arraySize; ++a) 
		{
			temparray[a] = m_array[a];
		}

		delete [] m_array;
		m_array = temparray;
	}
	else if (newsize == m_arraySize) 
	{
		// Do nothing
	}
}


template <class T>
inline void DArray<T>::Grow()
{
	// Double array size
	if (m_arraySize == 0)
	{
		SetSize(1);
	}
	else
	{
		SetSize(m_arraySize * 2);
	}
}


template <class T>
inline unsigned int DArray<T>::Push(const T &newdata)
{
    if (m_nextIndex == m_arraySize)			// Must resize the array
	{			 
		Grow();
    }
	    
    m_array[m_nextIndex] = newdata;
    m_nextIndex++;

    return m_nextIndex - 1;
}


template <class T>
inline T DArray<T>::Pop()
{
	if (m_nextIndex > 0)
	{
		m_nextIndex--;
		return m_array[m_nextIndex];
	}

	return T();
}


template <class T>
inline T DArray<T>::Top()
{
	if (m_nextIndex > 0)
	{
		return m_array[m_nextIndex-1];
	}

	return T();
}


template <class T>
void DArray<T>::Empty()
{
    delete [] m_array;
    m_array = NULL;
    m_arraySize = 0;
	m_nextIndex = 0;
}


template <class T>
void DArray<T>::EmptyAndDelete()
{
	for (unsigned int i = 0; i < m_nextIndex; ++i)
	{
		delete m_array[i];
	}

	Empty();
}


template <class T>
inline T DArray<T>::GetData(unsigned int index) const
{
	DebugAssert(index < m_nextIndex);
	return m_array[index];
}


template <class T>
inline T *DArray<T>::GetPointer(unsigned int index) const
{
    DebugAssert(index < m_nextIndex);
    return &(m_array[index]);
}


template <class T>
inline T DArray<T>::operator [] (unsigned int index)
{
    DebugAssert(index < m_nextIndex);
    return m_array[index];    
}


template <class T>
inline const T DArray<T>::operator [] (unsigned int index) const
{
    DebugAssert(index < m_nextIndex);
    return m_array[index];    
}


template <class T>
inline unsigned int DArray<T>::Size() const
{
    return m_nextIndex;
}
