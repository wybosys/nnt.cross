#include "cross.hpp"
#include "url.hpp"
#include <sstream>
#include "str.hpp"
#include "sys.hpp"

CROSS_BEGIN

Url::Url() {}

Url::Url(string const &str) {
    parse(str);
}

void Url::clear() {
    protocol.clear();
    host.clear();
    paths.clear();
    args.clear();
}

bool Url::parse(string const &str) {
    auto qss = explode(str, "?");

    auto lefts = explode(qss[0], "/", true);
    protocol = lefts[0];

    auto dms = explode(lefts[1], ":");
    if (dms.size() == 1) {
        host = dms[0];
        port = 0;
    } else {
        host = dms[0];
        port = (unsigned short) toint(dms[1]);
    }

    paths = ::std::vector<string>(lefts.begin() + 2, lefts.end());

    if (qss.size() > 1) {
        auto rights = explode(qss[1], "&");
        for (auto &e : rights) {
            auto kv = explode(e, "=");
            if (kv.size() != 2)
                continue;
            args[kv[0]] = kv[1];
        }
    }

    return true;
}

string Url::toString() const {
    ::std::vector<string> strs;
    if (!protocol.empty())
        strs.emplace_back(protocol + "/");
    if (!host.empty()) {
        if (port > 0) {
            strs.emplace_back(host + ":" + tostr(port));
        } else {
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

string Url::url() const {
    string r = host;
    if (port > 0)
        r += ":" + tostr(port);
    return r;
}

string build_querystring(Url::args_type const &args, fn_url_encoder url_encoder) {
    ::std::vector<string> strs;
    strs.reserve(args.size());

    for (auto &e : args) {
        ::std::ostringstream oss;
        oss << e.first << "=";
        if (url_encoder) {
            oss << url_encoder(e.second);
        } else {
            oss << e.second;
        }
        strs.emplace_back(oss.str());
    }

    return "?" + implode(strs, "&");
}

FormData::FormData()
{
    boundary = "----" + uuid();
}

void FormData::contenttype(string &ct) const
{
    ct = "multipart/form-data; boundary=" + boundary;
}

void FormData::body(buffer_type &buf) const
{
    for (auto const& e: args) {
        buf.write(boundary.c_str(), boundary.length());
    
        string header = "Content-Disposition: form-data; name=\"" + e.first + "\"\n";
        buf.write(header.c_str(), header.length());
        
        string val = e.second->toString();
        buf.write(val.c_str(), val.length());
        
        string eov = "\n\n";
        buf.write(eov.c_str(), eov.length());
    }
}

string FormData::contenttype() const
{
    string r;
    contenttype(r);
    return r;
}

auto FormData::body() const -> shared_ptr<buffer_type>
{
    auto r = make_shared<buffer_type>();
    body(*r);
    return r;
}

CROSS_END
