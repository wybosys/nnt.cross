#include "cross.h"
#include "str.h"

CROSS_BEGIN

vector<std::string> explode(string const &str, string const &delimeter, bool skipempty) {
    vector<std::string> ret;
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

string implode(vector<string> const &str, string const &delimeter) {
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

int toInt(string const &str) {
    return atoi(str.c_str());
}

float toFloat(string const &str) {
    return (float) atof(str.c_str());
}

double toDouble(string const &str) {
    return (double) atof(str.c_str());
}

CROSS_END
