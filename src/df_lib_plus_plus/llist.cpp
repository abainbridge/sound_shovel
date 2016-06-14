#define ReleaseAssert(a, b)
#define DebugAssert(a)

//*****************************************************************************
// Class LList
//*****************************************************************************

template <class T>
LList<T>::LList()
:	m_first(NULL),
    m_last(NULL),
    m_cached(NULL),
    m_cachedIndex(-1),
    m_numItems(0)
{
}


template <class T>
LList<T>::~LList()
{
    Empty();
}


template <class T>
void LList<T>::PutData(const T &data)
{
    PutDataAtEnd(data);
}


template <class T>
void LList<T>::PutDataAtEnd(const T &data)
{
    // Create the new data entry
    LListItem <T> *li = new LListItem <T> ();
    li->m_data = data;
    li->m_next = NULL;
    ++m_numItems;

    if (m_last == NULL) 
	{
        // No items added yet - this is the only one

        li->m_prev = NULL;
        m_first = li;
        m_last = li;

		m_cached = li;
		m_cachedIndex = 0;
    }
    else 
	{
        // Add the data onto the end
        m_last->m_next = li;
        li->m_prev = m_last;
        m_last = li;		
    }

	ReleaseAssert(m_cachedIndex >= 0 && m_cachedIndex < m_numItems, "Bad m_cachedIndex");
	ReleaseAssert(m_cached != NULL, "Bad m_cached");
}


template <class T>
void LList<T>::PutDataAtStart(const T &data)
{
    // Create the new data entry
    LListItem <T> *li = new LListItem <T> ();
    li->m_data = data;
    li->m_prev = NULL;
    ++m_numItems;

    if (m_last == NULL) 
	{
        // No items added yet - this is the only one

        li->m_next = NULL;
        m_first = li;
        m_last = li;

		m_cached = li;
		m_cachedIndex = 0;
    }
    else 
	{
        // Add the data onto the start
        m_first->m_prev = li;
        li->m_next = m_first;
        m_first = li;

		m_cachedIndex++;
    }

	ReleaseAssert(m_cachedIndex >= 0 && m_cachedIndex < m_numItems, "Bad m_cachedIndex");
	ReleaseAssert(m_cached != NULL, "Bad m_cached");
}


template <class T>
void LList<T>::PutDataAtIndex(const T &data, int index)
{
    if (index == 0) 
	{
        PutDataAtStart(data);
    }
    else if (index >= m_numItems) 
	{
        PutDataAtEnd(data);
    }
    else 
	{
        LListItem <T> *current = GetItem(index);

		ReleaseAssert(m_cachedIndex == index, "PutDataAtIndex: GetItem failed");

        // Create the new data entry
        LListItem <T> *li = new LListItem <T> ();
        li->m_data = data;		
        li->m_prev = current->m_prev;
        li->m_next = current;
        if (current->m_prev) current->m_prev->m_next = li;
        current->m_prev = li;		
        ++m_numItems;

		m_cached = li;
    }

	ReleaseAssert(m_cachedIndex >= 0 && m_cachedIndex < m_numItems, "Bad m_cachedIndex");
	ReleaseAssert(m_cached != NULL, "Bad m_cached");
}


template <class T>
int LList<T>::Size() const
{
    return m_numItems;
}


template <class T>
T *LList<T>::GetPointer(int index) const
{
    LListItem<T> *item = GetItem(index);
    if (item)
	    return &item->m_data;

	return NULL;
}


template <class T>
LListItem<T> *LList<T>::GetItem(int index) const
{
    if (!ValidIndex(index)) return NULL;


	//
	// Choose a place for which to start walking the list
	
	// Best place to start is either; m_first, m_cached or m_last
	//
	// m_first                m_cached                                    m_last
    //     |----------:-----------|---------------------:--------------------|
	//            mid-point 1                      mid-point 2
    //     
	// If index is less than mid-point 1, then m_first is nearest.
	// If index is greater than mid-point 2, then m_last is nearest.
	// Otherwise m_cached is nearest.
	// The two if statements below test for these conditions.


	if (index <= (m_cachedIndex >> 1))
	{
		// m_first is nearest target
		m_cached = m_first;
		m_cachedIndex = 0;
	}
	else if ((index - m_cachedIndex) > (m_numItems - index))
	{
		// m_last is nearest target
		m_cached = m_last;
		m_cachedIndex = m_numItems - 1;
	}
	// Otherwise m_cached is nearest


	//
	// Now walk up or down the list until we find our target
	
	while (index > m_cachedIndex) 
	{
        ReleaseAssert(m_cached->m_next, "List GetItem failed - m_cached->m_next == NULL");
		m_cached = m_cached->m_next;
		m_cachedIndex++;
	}
	while (index < m_cachedIndex) 
	{
        ReleaseAssert(m_cached->m_prev, "List GetItem failed - m_cached->m_prev == NULL");
		m_cached = m_cached->m_prev;
		m_cachedIndex--;
	}

    DebugAssert(m_cached);
	return m_cached;
}


template <class T>
T const LList<T>::operator [] (int index) const
{
    LListItem<T> const *item = GetItem(index);
    if (item)
        return item->m_data;

    return T();
}


template <class T>
bool LList<T>::ValidIndex(int index) const
{
    return (unsigned)index < m_numItems;
}


template <class T>
void LList<T>::Empty()
{
    LListItem <T> *current = m_first;
    while (current) 
	{
        LListItem <T> *m_next = current->m_next;
        delete current;
        current = m_next;
    }

    m_first = NULL;
    m_last = NULL;
    m_numItems = 0;
    m_cached = NULL;
    m_cachedIndex = -1;
}


template <class T>
void LList<T>::EmptyAndDelete()
{
    LListItem <T> *item = m_first;
    while (item)
    {
        delete item->m_data;
        item = item->m_next;
    }

    Empty();
}


template <class T>
void LList<T>::RemoveData(int index)
{
    LListItem <T> *current = GetItem(index);

    if (current == NULL)				return;
    
	if (current->m_prev == NULL)	    m_first = current->m_next;
    else								current->m_prev->m_next = current->m_next;
    
	if (current->m_next == NULL)		m_last = current->m_prev;
    else								current->m_next->m_prev = current->m_prev;

	// Are we about to remove the currently cached item?
	if (index == m_cachedIndex)
	{
		// Yes, so we must find a new valid item to cache

		if (m_numItems == 1)
		{
			m_cachedIndex = -1;  
			m_cached = NULL;
		}
		else if (index > 0)
		{
			m_cachedIndex--;
			m_cached = current->m_prev;
		}
		else
		{
			m_cached = current->m_next;
		}
	}
	else if (index < m_cachedIndex)
	{
		// Currently cached item is valid, but its index has changed
		m_cachedIndex--;
	}

	// Actually remove the target item
    delete current;
    --m_numItems;

	if (m_numItems > 0)
	{
		ReleaseAssert(m_cachedIndex >= 0 && m_cachedIndex < m_numItems, "Bad m_cachedIndex");
		ReleaseAssert(m_cached != NULL, "Bad m_cached");
	}
}


template <class T>
void LList<T>::RemoveDataAtEnd()
{
	RemoveData(m_numItems - 1);
}


template <class T>
int LList<T>::FindData(const T &data)
{
	int const size = Size();
    for (int i = 0; i < size; ++i) 
	{
        if ((*this)[i] == data) 
		{
            return i;
		}
	}

    return -1;
}

#undef ReleaseAssert
#undef DebugAssert
