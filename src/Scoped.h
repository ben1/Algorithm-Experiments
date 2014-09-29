#ifndef _Scoped_h_
#define _Scoped_h_


template<class T>
class Scoped
{
public:
    Scoped(T* a_pointer)
        : m_pointer(a_pointer)
    {
    }

    ~Scoped()
    {
        delete m_pointer;
    }

    operator T*() const { return m_pointer; }
    T* operator->() const { return m_pointer; }

private:
    T* m_pointer;
};


#endif
