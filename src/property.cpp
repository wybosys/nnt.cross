#include "cross.h"
#include "property.h"
#include <sstream>

CROSS_BEGIN

Property::VT FromCom(Property::variant::VT vt) {
    switch (vt) {
    case Property::variant::VT::INT:
    case Property::variant::VT::UINT:
    case Property::variant::VT::LONG:
    case Property::variant::VT::ULONG:
    case Property::variant::VT::SHORT:
    case Property::variant::VT::USHORT:
    case Property::variant::VT::LONGLONG:
    case Property::variant::VT::ULONGLONG:
    case Property::variant::VT::CHAR:
    case Property::variant::VT::UCHAR:
        return Property::VT::INTEGER;
    case Property::variant::VT::FLOAT:
    case Property::variant::VT::DOUBLE:
        return Property::VT::NUMBER;
    case Property::variant::VT::BOOLEAN:
        return Property::VT::BOOLEAN;
    case Property::variant::VT::STRING:
        return Property::VT::STRING;
    }
    return Property::VT::NIL;
}

Property::Property()
    : vt(VT::NIL) {
}

Property::Property(integer v)
    : vt(VT::INTEGER), _var(make_shared<variant>(v)) {
}

Property::Property(number v)
    : vt(VT::NUMBER), _var(make_shared<variant>(v)) {
}

Property::Property(bool v)
    : vt(VT::BOOLEAN), _var(make_shared<variant>(v)) {
}

Property::Property(string const &v)
    : vt(VT::STRING), _var(make_shared<variant>(v)) {
}

Property::Property(char const *v)
    : vt(VT::STRING), _var(make_shared<variant>(v)) {
}

Property::Property(Property::variant const &v)
    : vt(FromCom(v.vt)), _var(make_shared<variant>(v)) {
}

Property::Property(Property const& r)
    : vt(r.vt), _var(r._var), 
    name(r.name),
    declaration(r.declaration),
    _map(r._map),
    _array(r._array)
{
}

Property& Property::operator=(Property const&r) {
    const_cast<VT&>(vt) = r.vt;
    _var = r._var;

    name = r.name;
    declaration = r.declaration;
    _map = r._map;
    _array = r._array;

    return *this;
}

Property::integer Property::toInteger() const {
    switch (_var->vt) {
    case Property::variant::VT::INT:
        return _var->toInt();
    case Property::variant::VT::UINT:
        return _var->toUInt();
    case Property::variant::VT::LONG:
        return _var->toLong();
    case Property::variant::VT::ULONG:
        return _var->toULong();
    case Property::variant::VT::SHORT:
        return _var->toShort();
    case Property::variant::VT::USHORT:
        return _var->toUShort();
    case Property::variant::VT::LONGLONG:
        return (integer)_var->toLonglong();
    case Property::variant::VT::ULONGLONG:
        return (integer)_var->toULonglong();
    case Property::variant::VT::CHAR:
        return _var->toChar();
    case Property::variant::VT::UCHAR:
        return _var->toUChar();
    case Property::variant::VT::BOOLEAN:
        return _var->toBool();
    case Property::variant::VT::FLOAT:
        return (integer)round(_var->toFloat());
    case Property::variant::VT::DOUBLE:
        return (integer)round(_var->toDouble());
    }
    return 0;
}

Property::number Property::toNumber() const {
    switch (_var->vt) {
    case Property::variant::VT::FLOAT:
        return _var->toFloat();
    case Property::variant::VT::DOUBLE:
        return _var->toDouble();
    }
    return toInteger();
}

bool Property::toBool() const {
    if (_var->vt == Property::variant::VT::BOOLEAN)
        return _var->toBool();
    return toNumber() != 0;
}

string const &Property::toString() const {
    return _var->toString();
}

Property::map_type &Property::map() {
    if (vt == VT::MAP) {
        return *_map;
    }

    _var = nullptr;
    _array = nullptr;
    _map = make_shared<map_type>();
    return *_map;
}

Property::map_type const& Property::map() const {
    return *_map;
}

Property::array_type &Property::array() {
    if (vt == VT::ARRAY) {
        return *_array;
    }

    _var = nullptr;
    _map = nullptr;
    _array = make_shared<array_type>();
    return *_array;
}

Property::array_type const& Property::array() const {
    return *_array;
}


CROSS_END