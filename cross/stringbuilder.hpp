#ifndef __NNTCROSS_STRINGBUILDER_H_INCLUDED
#define __NNTCROSS_STRINGBUILDER_H_INCLUDED

#include <sstream>

class eol {
public:
    eol(::std::string const& s = "\n")
        : endl(s) {}
    ::std::string endl;
};

class stringbuilder
{
public:

    explicit stringbuilder(::std::string const& prefix = "", ::std::string const& suffix = "", ::std::string const& space = "")
        : _prefix(prefix), _suffix(suffix), _space(space)
    {}

    template <typename T>
    stringbuilder& add(T const& v) {
        if (_newline) {
            _oss << _prefix;
            _newline = false;
        }
        else {
            _oss << _space;
        }
        _oss << v;
        return *this;
    }

    template <typename T>
    inline stringbuilder& operator << (T const& v) {
        return add(v);
    }

    stringbuilder& operator << (eol const& v) {
        _oss << _suffix << v.endl;
        _newline = true;
        return *this;
    }
    
    stringbuilder& ln() {
        static eol t;
        *this << t;
        return *this;
    }

    inline operator ::std::string () const { 
        return _oss.str();
    }

    inline ::std::string str() const {
        return _oss.str();
    }
    
    inline stringbuilder& space(::std::string const& s) {
        _space = s;
        return *this;
    }
    
    inline stringbuilder& fix(::std::string const& pre, ::std::string const& suf) {
        _prefix = pre;
        _suffix = suf;
        return *this;
    }

private:
    
    bool _newline = true;
    ::std::string _prefix, _suffix, _space;
    ::std::ostringstream _oss;
};

template<typename _CharT, typename _Traits>
static ::std::basic_ostream <_CharT, _Traits> &operator<<(::std::basic_ostream <_CharT, _Traits> &stm, stringbuilder const &v) {
    stm << (::std::string)v;
    return stm;
}

#endif
