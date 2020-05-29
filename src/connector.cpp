#include "cross.h"
#include "connector.h"

CROSS_BEGIN

unsigned int Connector::CTIMEOUT = 30;
unsigned int Connector::TIMEOUT = 30;
string Connector::USERAGENT = "Mozilla/5.0 (Linux) AppleWebKit/600.1.4 (KHTML, like Gecko) NetType/WIFI";

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

string const& HttpConnector::send() const {
    return Nil<string>();
}

int HttpConnector::errcode() {
    return 0;
}

string const& HttpConnector::errmsg() {
    return Nil<string>();
}

string const& HttpConnector::body() {
    return Nil<string>();
}

HttpConnector::args_type const& HttpConnector::respheaders() {
    return _rspheaders;
}

CROSS_END