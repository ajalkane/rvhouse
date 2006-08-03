#ifndef _ACCESSOR_H_
#define _ACCESSOR_H_

template <class T>
class accessor {
    T _a;
public:
    inline accessor(T init) : _a(init) {}
    inline void instance(T s) { _a = s; }
    inline T operator()() { return _a; }
};

#endif //_ACCESSOR_H_
