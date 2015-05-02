#ifndef IU_CORE_UTILS_CORETYPES_H
#define IU_CORE_UTILS_CORETYPES_H

#include <string>
#define __STDC_CONSTANT_MACROS
#include <stdint.h>
//#include "Core/3rdpart/pstdint.h"
#include <memory>
#include "Core/Logging.h"
/*
#if _MSC_VER  && (_MSC_VER < 1800)
namespace std:: = std::tr1;
#else
namespace std:: = std;
#endif*/

/*#ifdef _MSC_VER
   typedef __int64 int64_t;
   typedef unsigned __int64 zuint64;
#else
   typedef long long int64_t;
   typedef unsigned long long zuint64;
#endif*/

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&);               \
    void operator=(const TypeName&)


// The ARRAY_SIZE(arr) macro returns the # of elements in an array arr.
// The expression is a compile-time constant, and therefore can be
// used in defining new arrays, for example.  If you use arraysize on
// a pointer by mistake, you will get a compile-time error.
//
// One caveat is that ARRAY_SIZE() doesn't accept any array of an
// anonymous type or a type defined inside a function.  In these rare
// cases, you have to use the unsafe ARRAYSIZE_UNSAFE() macro below.  This is
// due to a limitation in C++'s template system.  The limitation might
// eventually be removed, but it hasn't happened yet.

// This template function declaration is used in defining ARRAY_SIZE.
// Note that the function doesn't need an implementation, as we only
// use its type.
template <typename T, size_t N>
char (&ArraySizeHelper(T (&array)[N]))[N];

// That gcc wants both of these prototypes seems mysterious. VC, for
// its part, can't decide which to use (another mystery). Matching of
// template overloads: the final frontier.
#ifndef _MSC_VER
template <typename T, size_t N>
char (&ArraySizeHelper(const T (&array)[N]))[N];
#endif

#define ARRAY_SIZE(array) (sizeof(ArraySizeHelper(array)))

template <class T> struct EnumWrapper
{
    T value_;
    operator T&()
    {
        return value_;
    }

    T& operator =(const T& value)
    {
        value_ = value;
        return *this;
    }

    bool operator==(const T value)
    {
        return value_ == value;
    }
};

// std::shared_ptr release() implementation
// thx to http://stackoverflow.com/questions/1833356/detach-a-pointer-from-a-shared-ptr/5995770#5995770
// 
template <typename T>
class release_deleter{
public:
    release_deleter() : released_(new bool(false)){}
    void release() {*released_ = true;}
    void reset_released() { *released_ = false;}
    void operator()(T* ptr){
        if(!*released_)  {
            delete ptr;
        }
            
    }

private:
    //DISALLOW_COPY_AND_ASSIGN(release_deleter<T>);
    std::shared_ptr<bool> released_;
};

#ifndef Q_DECLARE_PRIVATE
    #define Q_DECLARE_PRIVATE(Class) \
        inline Class##Private* d_func() { return reinterpret_cast<Class##Private *>(d_ptr); } \
        inline const Class##Private* d_func() const { return reinterpret_cast<const Class##Private *>(d_ptr); } \
        friend class Class##Private;
    #define Q_D(Class) Class##Private * const d = d_func()

#define Q_DECLARE_PUBLIC(Class)                  \
  inline Class* q_func() { return static_cast<Class *>(q_ptr); } \
  inline const Class* q_func() const { return static_cast<const Class *>(q_ptr); } \
  friend class Class;

#define Q_Q(Class) Class * const q = q_func()
#endif

#define Q_DECLARE_PRIVATE_PTR(Class) \
        inline Class##Private* d_func() { return reinterpret_cast<Class##Private *>(d_ptr.get()); } \
        inline const Class##Private* d_func() const { return reinterpret_cast<const Class##Private *>(d_ptr.get()); } \
        friend class Class##Private;
#define Q_D(Class) Class##Private * const d = d_func()



#endif
