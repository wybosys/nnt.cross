#include "cross.h"
#include "connector.h"
#include <sstream>

CROSS_BEGIN

unsigned int Connector::CTIMEOUT = 30;
unsigned int Connector::TIMEOUT = 30;
string Connector::USERAGENT = "Mozilla/5.0 (Linux) AppleWebKit/600.1.4 (KHTML, like Gecko) NetType/WIFI";

HttpConnector& HttpConnector::setarg(string const& key, arg_type const& arg) {
    _reqargs[key] = arg;
    return *this;
}

HttpConnector& HttpConnector::setargs(args_type const& args) {
    for (auto& e : args) {
        _reqargs[e.first] = e.second;
    }
    return *this;
}

HttpConnector::arg_type const& HttpConnector::getarg(string const& key) {
    auto fnd = _reqargs.find(key);
    return fnd != _reqargs.end() ? fnd->second : Nil<arg_type>();
}

bool HttpConnector::hasarg(string const& key) {
    auto fnd = _reqargs.find(key);
    return fnd != _reqargs.end();
}

HttpConnector& HttpConnector::setheader(string const& key, arg_type const& header) {
    _reqheaders[key] = header;
    return *this;
}

HttpConnector& HttpConnector::setheaders(args_type const& headers) {
    for (auto& e : headers) {
        _reqheaders[e.first] = e.second;
    }
    return *this;
}

HttpConnector::arg_type const& HttpConnector::getheader(string const& key) {
    auto fnd = _reqheaders.find(key);
    return fnd != _reqheaders.end() ? fnd->second : Nil<arg_type>();
}

bool HttpConnector::hasheader(string const& key) {
    auto fnd = _reqheaders.find(key);
    return fnd != _reqheaders.end();
}

bool HttpConnector::uploads(files_type const& files) {
    return false;
}

bool HttpConnector::send() const {
    return false;
}

int HttpConnector::errcode() const {
    return 0;
}

string const& HttpConnector::errmsg() const {
    return Nil<string>();
}

stringbuf const& HttpConnector::body() const {
    return *Nil<ostringstream>().rdbuf();
}

HttpConnector::args_type const& HttpConnector::respheaders() const {
    return Nil<args_type>();
}

unsigned short HttpConnector::respcode() const {
    return 404;
}

Connector::arg_type Combine(Connector::args_type const& args) {
    auto r = make_shared<Property>();
    r->map();
    for (auto &e : args) {
        auto &v = e.second;
        if (v->name.empty())
            v->name = e.first;
        r->map()[e.first] = v;
    }
    return r;
}

CROSS_END