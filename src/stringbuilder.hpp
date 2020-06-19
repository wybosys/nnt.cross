#ifndef __NNTCROSS_STRINGBUILDER_H_INCLUDED
#define __NNTCROSS_STRINGBUILDER_H_INCLUDED

#include <sstream>

class stringbuilder
{
public:

    explicit stringbuilder(::std::string const& space = "")
        : _space(space) {}

    template <typename T>
    inline stringbuilder& operator << (T const& v) {
        _oss << v << _space;
        return *this;
    }

    inline operator ::std::string() const {
        return _oss.str();
    }

private:
    ::std::string _space;
    ::std::ostringstream _oss;
};

template<typename _CharT, typename _Traits>
static ::std::basic_ostream <_CharT, _Traits> &operator<<(::std::basic_ostream <_CharT, _Traits> &stm, stringbuilder const &v) {
    stm << v._oss.str();
    return stm;
}

#endif