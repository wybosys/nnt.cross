#include "cross.h"
#include "str.h"

CROSS_BEGIN

vector<std::string> explode(string const& str, string const& delimeter, bool skipempty)
{
    vector<std::string> ret;
    rsize_t sid = 0, eid = 0;
    while ((eid = str.find(delimeter, sid)) < str.size())
    {
        string val = str.substr(sid, eid - sid);
        if (!skipempty || val.length())
            ret.push_back(val);
        sid = eid + delimeter.size();
    }
    if (sid < str.size())
    {
        string val = str.substr(sid);
        if (!skipempty || val.length())
            ret.push_back(val);
    }
    return ret;
}

string implode(vector<string> const& str, string const& delimeter)
{
    string r;
    for (auto& i : str)
        r += delimeter + i;
    return r;
}

bool beginwith(string const& str, string const& tgt)
{
    if (str.length() < tgt.length())
        return false;
    return str.substr(0, tgt.length()) == tgt;
}

bool endwith(string const& str, string const& tgt)
{
    if (str.length() < tgt.length())
        return false;
    return str.substr(str.length() - tgt.length(), tgt.length()) == tgt;
}

int toInt(string const& str)
{
    return atoi(str.c_str());
}

float toFloat(string const& str) {
    return (float)atof(str.c_str());
}

double toDouble(string const& str) {
    return (double)atof(str.c_str());
}

CROSS_END
