#pragma once


// This is a sort-of vector-list hybrid.
// List like features:
//  * You can add and remove data to/from anywhere
//
// Vector like features:
//  * You can access items by index, eg foo[12]. This is fast if the last item
//    accessed was near 12.


template <class T>
struct LListItem
{
    T		  m_data;
    LListItem *m_next;
    LListItem *m_prev;
};


template <class T>
class LList
{
protected:
    LListItem<T> *m_first;
	LListItem<T> *m_last;

	mutable LListItem<T> *m_cached;      // \ Used to get quick access to items near the
	mutable int m_cachedIndex;           // / last item that was accessed (common)

	int         m_numItems;

protected:
	inline LListItem<T> *GetItem(int index) const;

public:
    LList();
	~LList();

	inline void PutData			(const T &data);        // Adds data at the end
	void		PutDataAtEnd	(const T &data);
	void		PutDataAtStart	(const T &data);
	void		PutDataAtIndex	(const T &data, int index);

    inline T   *GetPointer		(int index) const;	    // \ O(n) unless sequential
    inline T const operator []  (int index) const;      // /

    void		RemoveData		(int index);			// O(n) unless sequential
	inline void	RemoveDataAtEnd	();

    int			FindData		(const T &data);		// -1 means 'not found'

    inline int	Size			() const;				// Returns the total size of the array
    inline bool ValidIndex		(int index) const;

    void		Empty			();						// Resets the array to empty
    void		EmptyAndDelete	();						// As above, deletes all data as well

};


#include "llist.cpp"
