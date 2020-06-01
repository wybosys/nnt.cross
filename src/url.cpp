#include "cross.hpp"
#include "url.hpp"
#include <sstream>
#include "str.hpp"

CROSS_BEGIN

Url::Url() {}

Url::Url(string const& str)
{
    parse(str);
}

void Url::clear()
{
    protocol.clear();
    host.clear();
    paths.clear();
    args.clear();
}

bool Url::parse(string const& str)
{
    auto qss = explode(str, "?");
    if (qss.size() != 2)
        return false;

    auto lefts = explode(qss[0], "/", true);
    protocol = lefts[0];

    auto dms = explode(lefts[1], ":");
    if (dms.size() == 1) {
        host = dms[0];
        port = 0;
    }
    else {
        host = dms[0];
        port = (unsigned short)toint(dms[1]);
    }

    paths = ::std::vector<string>(lefts.begin() + 2, lefts.end());

    auto rights = explode(qss[1], "&");
    for (auto &e : rights) {
        auto kv = explode(e, "=");
        if (kv.size() != 2)
            continue;
        args[kv[0]] = kv[1];
    }

    return true;
}

string Url::toString() const
{
    ::std::vector<string> strs;
    if (!protocol.empty())
        strs.emplace_back(protocol + "/");
    if (!host.empty()) {
        if (port > 0) {
            strs.emplace_back(host + ":" + tostr(port));
        }
        else {
            strs.emplace_back(host);
        }
    }
    if (!paths.empty())
        strs.emplace_back(implode(paths, "/"));
    string r = implode(strs, "/");
    if (!args.empty()) {
        r += build_querystring(args);
    }
    return r;
}

string Url::path() const {
    return "/" + implode(paths, "/");
}

string build_querystring(Url::args_type const& args, fn_url_encoder url_encoder)
{
    ::std::vector<string> strs;
    strs.reserve(args.size());

    for (auto &e : args) {
        ::std::ostringstream oss;
        oss << e.first << "=";
        if (url_encoder) {
            oss << url_encoder(e.second);
        }
        else {
            oss << e.second;
        }      
        strs.emplace_back(oss.str());
    }

    return "?" + implode(strs, "&");
}

CROSS_END