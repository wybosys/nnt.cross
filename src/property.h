#ifndef __NNTCROSS_PROPERTY_H_INCLUDED
#define __NNTCROSS_PROPERTY_H_INCLUDED

#include <map>
#include "com++.h"

CROSS_BEGIN

class Property
{
public:

    enum struct VT {
        NIL,
        INTEGER,
        NUMBER,
        BOOLEAN,
        STRING
    };

    typedef ptrdiff_t integer;
    typedef double number;
    typedef COMXX_NS::Variant variant;

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

    string const &toString() const;

    operator integer() const;

    operator number() const;

    operator bool() const;

    operator string const& () const;

    operator variant const &() const;

    Property& operator=(Property const&);

private:
    variant const _var;
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

inline Property::operator string const& () const {
    return toString();
}

inline Property::operator variant const &() const {
    return _var;
}

template<typename _CharT, typename _Traits>
static basic_ostream <_CharT, _Traits> &operator<<(basic_ostream <_CharT, _Traits> &stm, Property const &v) {
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
static basic_ostream <_CharT, _Traits> &operator<<(basic_ostream <_CharT, _Traits> &stm, shared_ptr <Property> const &v) {
    if (!v)
        return stm;
    return stm << *v;
}

CROSS_END

#endif