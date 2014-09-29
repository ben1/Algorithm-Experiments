#ifndef _ScopedArray_h_
#define _ScopedArray_h_


template<class T>
class ScopedArray
{
public:
    ScopedArray(T* a_pointer)
        : m_pointer(a_pointer)
    {
    }

    ~ScopedArray()
    {
        delete [] m_pointer;
    }

    operator T*() const { return m_pointer; }
    T* operator()() const { return m_pointer; }
    T& operator[](int a_index) const { return m_pointer[a_index]; }

private:
    T* m_pointer;
};


#endif
