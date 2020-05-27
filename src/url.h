#ifndef __NNTCROSS_URL_H_INCLUDED
#define __NNTCROSS_URL_H_INCLUDED

#include <map>
#include "com++.h"

CROSS_BEGIN

class UrlValue
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

    UrlValue();

    UrlValue(integer);

    UrlValue(number);

    UrlValue(bool);

    UrlValue(string const &);

    UrlValue(char const *);

    UrlValue(variant const &);

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

private:
    variant const _var;
};

inline UrlValue::operator integer() const {
    return toInteger();
}

inline UrlValue::operator number() const {
    return toNumber();
}

inline UrlValue::operator bool() const {
    return toBool();
}

inline UrlValue::operator string const& () const {
    return toString();
}

inline UrlValue::operator variant const &() const {
    return _var;
}

template<typename _CharT, typename _Traits>
static basic_ostream <_CharT, _Traits> &operator<<(basic_ostream <_CharT, _Traits> &stm, UrlValue const &v) {
    switch (v.vt) {
    case UrlValue::VT::STRING:
        stm << v.toString();
        break;
    case UrlValue::VT::INTEGER:
        stm << v.toInteger();
        break;
    case UrlValue::VT::NUMBER:
        stm << v.toNumber();
        break;
    case UrlValue::VT::BOOLEAN:
        stm << v.toBool();
        break;
    }
    return stm;
}

template<typename _CharT, typename _Traits>
static basic_ostream <_CharT, _Traits> &operator<<(basic_ostream <_CharT, _Traits> &stm, shared_ptr <UrlValue> const &v) {
    if (!v)
        return stm;
    return stm << *v;
}

CROSS_END

#endif