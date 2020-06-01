#ifndef __NNTCROSS_PROPERTY_H_INCLUDED
#define __NNTCROSS_PROPERTY_H_INCLUDED

#include <map>
#include "com++.hpp"

CROSS_BEGIN

// key的类型
class PropertyKey
{
public:
    
    enum struct VT {
        INTEGER,
        NUMBER,
        STRING,
        BOOLEAN
    };

    const VT vt;

    typedef ptrdiff_t integer;
    typedef double number;

    PropertyKey(integer v);
    PropertyKey(number);
    PropertyKey(string const&);
    PropertyKey(bool);

    inline bool operator == (PropertyKey const& r) const {
        return _k == r._k;
    }

    inline bool operator != (PropertyKey const& r) const {
        return _k == r._k;
    }

    inline bool operator < (PropertyKey const& r) const {
        return _k < r._k;
    }

    inline bool operator <= (PropertyKey const& r) const {
        return _k <= r._k;
    }

    inline bool operator > (PropertyKey const& r) const {
        return _k > r._k;
    }

    inline bool operator >= (PropertyKey const& r) const {
        return _k >= r._k;
    }

    operator integer() const;
    operator number() const;
    operator string const& () const;
    operator bool() const;

private:
    union {
        integer i;
        number n;
        bool b;
    } _pod = { 0 };
    string _k;
};

// 定义域
class PropertyDeclaration 
{
public:

    // 约束
    string ns, scheme;

    // 编码
    string encoding;
};

class Property
{
public:

    enum struct VT {
        NIL,
        INTEGER,
        NUMBER,
        BOOLEAN,
        STRING,
        ARRAY,
        MAP
    };

    typedef ptrdiff_t integer;
    typedef double number;
    typedef COMXX_NS::Variant variant;
    typedef shared_ptr<Property> data_type;
    typedef ::std::map<PropertyKey, data_type> map_type;
    typedef ::std::vector<data_type> array_type;

    Property();

    Property(integer);

    Property(number);

    Property(bool);

    Property(string const &);

    Property(char const *);

    Property(variant const &);

    Property(Property const&);

    VT const vt;

    integer toInteger() const;

    number toNumber() const;

    bool toBool() const;

    string toString() const;

    operator integer() const;

    operator number() const;

    operator bool() const;

    operator string () const;

    operator variant const &() const;

    Property& operator=(Property const&);

    // 名称
    string name;

    // 定义域
    shared_ptr<PropertyDeclaration> declaration;
    
    // 转换成map
    map_type &map();

    // 获取map
    map_type const& map() const;

    // 获取map中的值
    data_type map(string const&) const;

    // 转换成array
    array_type &array();

    // 获取array
    array_type const& array() const;

    // 获取array的下标对应的值
    data_type array(size_t) const;

private:
    shared_ptr<variant> _var;
    shared_ptr<map_type> _map;
    shared_ptr<array_type> _array;
};

inline Property::operator integer() const {
    return toInteger();
}

inline Property::operator number() const {
    return toNumber();
}

inline Property::operator bool() const {
    return toBool();
}

inline Property::operator string () const {
    return toString();
}

inline Property::operator variant const &() const {
    return *_var;
}

inline Property::data_type Property::map(string const& key) const {
    auto &m = map();
    auto fnd = m.find(key);
    return fnd == m.end() ? nullptr : fnd->second;
}

inline Property::data_type Property::array(size_t idx) const {
    auto &l = array();
    return idx < l.size() ? l[idx] : nullptr;
}

template<typename _CharT, typename _Traits>
static ::std::basic_ostream <_CharT, _Traits> &operator<<(::std::basic_ostream <_CharT, _Traits> &stm, Property const &v) {
    switch (v.vt) {
    case Property::VT::STRING:
        stm << v.toString();
        break;
    case Property::VT::INTEGER:
        stm << v.toInteger();
        break;
    case Property::VT::NUMBER:
        stm << v.toNumber();
        break;
    case Property::VT::BOOLEAN:
        stm << v.toBool();
        break;
    }
    return stm;
}

template<typename _CharT, typename _Traits>
static ::std::basic_ostream <_CharT, _Traits> &operator<<(::std::basic_ostream <_CharT, _Traits> &stm, shared_ptr <Property> const &v) {
    if (!v)
        return stm;
    return stm << *v;
}

template <typename V>
inline shared_ptr<Property> make_property(V& v) {
    return make_shared<Property>(v);
}

CROSS_END

#endif