#include "cross.hpp"
#include "str.hpp"
#include <sstream>
#include <regex>

CROSS_BEGIN

strings explode(string const &str, string const &delimeter, bool skipempty) {
    strings ret;
    size_t sid = 0, eid = 0;
    while ((eid = str.find(delimeter, sid)) < str.size()) {
        string val = str.substr(sid, eid - sid);
        if (!skipempty || val.length())
            ret.push_back(val);
        sid = eid + delimeter.size();
    }
    if (sid < str.size()) {
        string val = str.substr(sid);
        if (!skipempty || val.length())
            ret.push_back(val);
    }
    return ret;
}

string implode(strings const &str, string const &delimeter) {
    const size_t l = str.size();
    if (l) {
        string r = str[0];
        for (auto i = str.begin() + 1; i != str.end(); ++i)
            r += delimeter + *i;
        return r;
    }
    return "";
}

bool beginwith(string const &str, string const &tgt) {
    if (str.length() < tgt.length())
        return false;
    return str.substr(0, tgt.length()) == tgt;
}

bool endwith(string const &str, string const &tgt) {
    if (str.length() < tgt.length())
        return false;
    return str.substr(str.length() - tgt.length(), tgt.length()) == tgt;
}

template<typename T>
inline T fromstr(string const &str) {
    ::std::istringstream iss(str);
    T v;
    iss >> v;
    return v;
}

int toint(string const &str) {
    return fromstr<int>(str);
}

float tofloat(string const &str) {
    return fromstr<float>(str);
}

double todouble(string const &str) {
    return fromstr<double>(str);
}

template<typename T>
inline string tostr(T v) {
    ::std::ostringstream oss;
    oss << v;
    return oss.str();
}

string tostr(int v) {
    return tostr<int>(v);
}

string tostr(float v) {
    return tostr<float>(v);
}

string tostr(double v) {
    return tostr<double>(v);
}

string replace(string const &str, string const &pat, string const &tgt) {
    return ::std::regex_replace(str, ::std::regex(pat), tgt);
}

CROSS_END
