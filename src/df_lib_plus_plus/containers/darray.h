#pragma once

// Like std::vector but:
// * Doesn't use exceptions.
// * Doesn't support a custom allocator.
// * Doesn't support iterators.
// * The empty() function empties the array, unlike with std::vector, where it
//   indicates if it is empty. With std::vector it is easy to call empty() 
//   believing it will empty the array. With DArray, if you call empty() 
//   expecting a result indicating whether the array is empty, you'll get a 
//   compile error (because the return type is void).
// * Capitalized function names.
// * Doesn't use the std namespace.
// * Resize can be used to reduce the capacity. No need for shrink_to_fit().
// * Generates smaller code.
// * Gives slightly better compile errors.

template <class T>
class DArray
{
protected:
    unsigned m_capacity;        // Size of m_array.
    unsigned m_size;            // Number of valid items in m_array.

    inline void Grow() { SetCapacity(1 + (unsigned)(m_capacity * 1.5)); }

public:
    T       *m_array;

    DArray();
    DArray(DArray <T> const &other);
    ~DArray() { Empty(); }

    void SetCapacity(unsigned newCapacity);
    inline void Resize(unsigned newSize);

    inline unsigned Push(const T &newData); // Returns index used.
    inline T Pop();                         // Returns head item and removes it.
    inline T Top();                         // Returns head item.

    inline unsigned Size() const { return m_size; } // Returns the number of used entries.

    void Empty();                           // Resets the array to empty.
    void EmptyAndDelete();                  // Same as Empty() but deletes the elements that are pointed to as well.

    inline T &operator[] (unsigned index);
    inline const T &operator[] (unsigned index) const;
};


#include "darray.cpp"
