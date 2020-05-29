#ifndef __NNT_MACROXX_H_INCLUDED
#define __NNT_MACROXX_H_INCLUDED

#ifndef NNT_NS
#define NNT_NS nnt
#endif

#define NNT_BEGIN    \
    namespace NNT_NS \
    {
#define NNT_END }

#define USE_NNT using namespace NNT_NS;
#define USE_NNT_NS(ns) \
    USE_NNT;           \
    using namespace NNT_NS::ns;

#define NNT_BEGIN_NS(ns)   \
    NNT_BEGIN namespace ns \
    {
#define NNT_END_NS() \
    }                \
    NNT_END

#define NNT_FRIEND(cls) friend class cls;
#define NNT_NOCOPY(cls) \
private:                \
    cls(cls &) = delete;
#define NNT_PRIVATECLASS(cls) cls##Private

#define NNT_CLASS_PREPARE(cls) \
    class cls;                 \
    class NNT_PRIVATECLASS(cls);

#define NNT_CLASS_DECL(cls)                                               \
protected:                                                                \
    typedef cls self_class_type;                                          \
    typedef NNT_PRIVATECLASS(cls) private_class_type;                     \
    friend class NNT_PRIVATECLASS(cls);                                   \
    private_class_type *d_ptr = nullptr;                                  \
    friend private_class_type *nnt::DPtr<cls, private_class_type>(cls *); \
                                                                          \
private:                                                                  \
    NNT_NOCOPY(cls);

#define NNT_CLASS_CONSTRUCT(...) \
    d_ptr = new private_class_type(__VA_ARGS__);
#define NNT_CLASS_DESTORY() \
    delete d_ptr;           \
    d_ptr = nullptr;

#define NNT_SINGLETON_DECL(cls) \
public:                         \
    static cls &shared();       \
    static void free_shared();  \
    static bool is_shared();
#define NNT_SINGLETON_IMPL(cls, ...)                \
    static cls *_##cls##_shared = nullptr;          \
    cls &cls::shared()                              \
    {                                               \
        if (!_##cls##_shared)                       \
        {                                           \
            _##cls##_shared = new cls(__VA_ARGS__); \
        }                                           \
        return *_##cls##_shared;                    \
    }                                               \
    void cls::free_shared()                         \
    {                                               \
        if (_##cls##_shared)                        \
        {                                           \
            delete _##cls##_shared;                 \
            _##cls##_shared = nullptr;              \
        }                                           \
    }                                               \
    bool cls::is_shared() { return nullptr != _##cls##_shared; }

#define NNT_THIS const_cast<self_class_type *>(this)
#define NNT_DPTR const_cast<private_class_type *>(d_ptr)

#define NNT_DOT .
#define NNT_COMMA ,

#define __NNT_RAW(L) L
#define __NNT_COMBINE(L, R) L##R
#define _NNT_COMBINE(L, R) __NNT_COMBINE(L, R)

#define NNT_AUTOGUARD(obj, ...) ::std::lock_guard<::std::mutex> _NNT_COMBINE(__auto_guard_, __LINE__)(obj);

#define NNT_PASS
#define NNT_PASS_EXPRESS(...) \
    {                         \
        __VA_ARGS__           \
    }                         \
    while (0)

#ifdef NNT_STATIC
#define NNT_LIBRARY 1
#endif

#ifdef NNT_SHARED
#define NNT_LIBRARY 1
#endif

#ifndef NNT_LIBRARY
#define NNT_APP 1
#endif

#if defined(WIN32) || defined(_WIN32)
#include <SDKDDKVer.h>
#pragma warning(disable : 4251)
#define WIN32_LEAN_AND_MEAN
#define NNT_WINDOWS
#ifdef NNT_LIBRARY
#ifdef NNT_SHARED
#define NNT_API __declspec(dllexport)
#endif
#elif !defined(NNT_USE_STATIC)
#define NNT_API __declspec(dllimport)
#endif
#else
#define NNT_UNIXLIKE
#endif

#ifndef NNT_API
#define NNT_API NNT_PASS
#endif

#include <string>
#include <iostream>
#include <vector>
#include <atomic>

#if defined(NNT_WINDOWS) && defined(_UNICODE)
#include <xstring>
#endif

#if defined(DEBUG) || defined(_DEBUG)
#define NNT_DEBUG
#define NNT_DEBUG_SYMBOL(exp) exp
#define NNT_RELEASE_SYMBOL(exp)
#define NNT_DEBUG_EXPRESS(exp) NNT_PASS_EXPRESS(exp)
#define NNT_RELEASE_EXPRESS(exp) NNT_PASS_EXPRESS()
#else
#define NNT_RELEASE
#define NNT_DEBUG_SYMBOL(exp)
#define NNT_RELEASE_SYMBOL(exp) exp
#define NNT_DEBUG_EXPRESS(exp) NNT_PASS_EXPRESS()
#define NNT_RELEASE_EXPRESS(exp) NNT_PASS_EXPRESS(exp)
#endif

NNT_BEGIN

using namespace ::std;

#if defined(NNT_WINDOWS) && defined(_UNICODE)
typedef wstring system_string;
#else
typedef string system_string;
#endif

template <class T, class TP = typename T::private_class_type>
static TP *DPtr(T *obj)
{
    return obj->d_ptr;
}

template <typename T>
static T const &Nil()
{
    static const T __s;
    return __s;
};

class Object {
public:
    virtual ~Object() = default;
};

typedef Object IObject;

class RefObject : public IObject {
public:

    virtual void grab() const {
        ++_referencedCount;
    }

    virtual bool drop() const {
        if (--_referencedCount == 0) {
            delete this;
            return true;
        }
        return false;
    }

private:
    mutable atomic<size_t> _referencedCount = 1;
};

template <typename T>
class shared_ref
{
public:

    shared_ref()
        : _ptr(new T())
    {
    }

    shared_ref(shared_ref<T> const& r)
        : _ptr(const_cast<T*>(r.get()))
    {
        if (_ptr)
            _ptr->grab();
    }

    template <typename R>
    shared_ref(shared_ref<R> const& r) 
        : _ptr(dynamic_cast<T*>(const_cast<R*>(r.get())))
    {
        if (_ptr)
            _ptr->grab();
    }

    ~shared_ref() {
        if (_ptr) {
            _ptr->drop();
            _ptr = nullptr;
        }
    }

    shared_ref<T>& operator = (T const* r) {
        if (_ptr == r)
            return *this;
        if (_ptr)
            _ptr->drop();
        _ptr = const_cast<T*>(r);
        if (_ptr)
            _ptr->grab();
        return *this;
    }

    template <typename R>
    inline operator R* () {
        return dynamic_cast<R*>(_ptr);
    }

    template <typename R>
    inline operator R const* () const {
        return dynamic_cast<R const*>(_ptr);
    }
    
    inline T& operator * () {
        return *_ptr;
    }

    inline T const& operator *() const {
        return *_ptr;
    }

    inline T* operator -> () {
        return _ptr;
    }

    inline T const* operator -> () const {
        return _ptr;
    }

    inline bool operator < (shared_ref<T> const& r) const {
        return _ptr < r._ptr;
    }

    inline T* get() {
        return _ptr;
    }

    inline T const* get() const {
        return _ptr;
    }

private:
    T* _ptr;

    shared_ref(nullptr_t) :_ptr(nullptr) {}

    static shared_ref<T> _assign(T* ptr) {
        auto r = shared_ref<T>(nullptr);
        r._ptr = ptr;
        return r;
    }

    template <typename T, typename ...Args> friend shared_ref<T> make_ref(Args&&...);
};

template <typename T, typename ...Args>
static shared_ref<T> make_ref(Args&&... args)
{
    return shared_ref<T>::_assign(new T(forward<Args>(args)...));
};

NNT_END

#endif
