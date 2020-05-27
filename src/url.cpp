#include "cross.h"
#include "url.h"
#include <strstream>

CROSS_BEGIN

UrlValue::VT FromCom(UrlValue::variant::VT vt) {
    switch (vt) {
    case UrlValue::variant::VT::INT:
    case UrlValue::variant::VT::UINT:
    case UrlValue::variant::VT::LONG:
    case UrlValue::variant::VT::ULONG:
    case UrlValue::variant::VT::SHORT:
    case UrlValue::variant::VT::USHORT:
    case UrlValue::variant::VT::LONGLONG:
    case UrlValue::variant::VT::ULONGLONG:
    case UrlValue::variant::VT::CHAR:
    case UrlValue::variant::VT::UCHAR:
        return UrlValue::VT::INTEGER;
    case UrlValue::variant::VT::FLOAT:
    case UrlValue::variant::VT::DOUBLE:
        return UrlValue::VT::NUMBER;
    case UrlValue::variant::VT::BOOLEAN:
        return UrlValue::VT::BOOLEAN;
    case UrlValue::variant::VT::STRING:
        return UrlValue::VT::STRING;
    }
    return UrlValue::VT::NIL;
}

UrlValue::UrlValue()
    : vt(VT::NIL) {
}

UrlValue::UrlValue(integer v)
    : vt(VT::INTEGER), _var(v) {
}

UrlValue::UrlValue(number v)
    : vt(VT::NUMBER), _var(v) {
}

UrlValue::UrlValue(bool v)
    : vt(VT::BOOLEAN), _var(v) {
}

UrlValue::UrlValue(string const &v)
    : vt(VT::STRING), _var(v) {
}

UrlValue::UrlValue(char const *v)
    : vt(VT::STRING), _var(v) {
}

UrlValue::UrlValue(UrlValue::variant const &v)
    : vt(FromCom(v.vt)), _var(v) {
}

UrlValue::UrlValue(UrlValue const& r)
    : vt(r.vt), _var(r._var) {
}

UrlValue& UrlValue::operator=(UrlValue const&r) {
    const_cast<VT&>(vt) = r.vt;
    const_cast<variant&>(_var) = r._var;
    return *this;
}

UrlValue::integer UrlValue::toInteger() const {
    switch (_var.vt) {
    case UrlValue::variant::VT::INT:
        return _var.toInt();
    case UrlValue::variant::VT::UINT:
        return _var.toUInt();
    case UrlValue::variant::VT::LONG:
        return _var.toLong();
    case UrlValue::variant::VT::ULONG:
        return _var.toULong();
    case UrlValue::variant::VT::SHORT:
        return _var.toShort();
    case UrlValue::variant::VT::USHORT:
        return _var.toUShort();
    case UrlValue::variant::VT::LONGLONG:
        return (integer)_var.toLonglong();
    case UrlValue::variant::VT::ULONGLONG:
        return (integer)_var.toULonglong();
    case UrlValue::variant::VT::CHAR:
        return _var.toChar();
    case UrlValue::variant::VT::UCHAR:
        return _var.toUChar();
    case UrlValue::variant::VT::BOOLEAN:
        return _var.toBool();
    case UrlValue::variant::VT::FLOAT:
        return (integer)round(_var.toFloat());
    case UrlValue::variant::VT::DOUBLE:
        return (integer)round(_var.toDouble());
    }
    return 0;
}

UrlValue::number UrlValue::toNumber() const {
    switch (_var.vt) {
    case UrlValue::variant::VT::FLOAT:
        return _var.toFloat();
    case UrlValue::variant::VT::DOUBLE:
        return _var.toDouble();
    }
    return toInteger();
}

bool UrlValue::toBool() const {
    if (_var.vt == UrlValue::variant::VT::BOOLEAN)
        return _var.toBool();
    return toNumber() != 0;
}

string const &UrlValue::toString() const {
    return _var.toString();
}

CROSS_END