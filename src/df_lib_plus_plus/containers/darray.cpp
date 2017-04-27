// Project headers
#include "df_common.h"

// Standard headers
#include <stdlib.h>


template <class T>
DArray<T>::DArray()
{
    m_capacity = 0;
    m_size = 0;
    m_array = NULL;
}


template <class T>
DArray<T>::DArray(DArray <T> const &other)
{
    m_capacity = other.m_capacity;
    m_size = other.m_size;
    if (m_capacity > 0)
    {
        m_array = new T[m_capacity];
        for (int i = 0; i < m_capacity; i++)
            m_array[i] = other.m_array[i];
    }
    else
    {
        m_array = NULL;
    }
}


template <class T>
inline void DArray<T>::SetCapacity(unsigned newCapacity)
{
	if (newCapacity > m_capacity)
	{
		unsigned oldCapacity = m_capacity;
		m_capacity = newCapacity;
		T *tempArray = new T[m_capacity];

		for (unsigned a = 0; a < oldCapacity; a++)
			tempArray[a] = m_array[a];

		delete[] m_array;
		m_array = tempArray;
	}
	else if (newCapacity < m_capacity)
	{
		m_capacity = newCapacity;
		T *tempArray = new T[m_capacity];

		for (unsigned a = 0; a < m_capacity; a++)
			tempArray[a] = m_array[a];

		delete[] m_array;
		m_array = tempArray;
	}
}


template <class T>
inline void DArray<T>::Resize(unsigned newSize)
{
    SetCapacity(newSize);
    m_size = newSize;
}


template <class T>
inline unsigned DArray<T>::Push(const T &newData)
{
    if (m_size == m_capacity)
		Grow();

    m_array[m_size] = newData;
    m_size++;

    return m_size - 1;
}


template <class T>
inline T DArray<T>::Pop()
{
	if (m_size > 0)
	{
		m_size--;
		return m_array[m_size];
	}

	return T();
}


template <class T>
inline T DArray<T>::Top()
{
	if (m_size > 0)
		return m_array[m_size-1];

	return T();
}


template <class T>
void DArray<T>::Empty()
{
    delete[] m_array;
    m_array = NULL;
    m_capacity = 0;
	m_size = 0;
}


template <class T>
void DArray<T>::EmptyAndDelete()
{
	for (unsigned i = 0; i < m_size; i++)
		delete m_array[i];

	Empty();
}


template <class T>
inline T &DArray<T>::operator[] (unsigned index)
{
    DebugAssert(index < m_nextIndex);
    return m_array[index];
}


template <class T>
inline const T &DArray<T>::operator[] (unsigned index) const
{
    DebugAssert(index < m_nextIndex);
    return m_array[index];
}
