#include "cross.hpp"
#include "connector_curl.hpp"
#include "str.hpp"
#include <sstream>
#include <fstream>
#include "json.hpp"
#include "xml.hpp"
#include "memory.hpp"
#include <string.h>
#include <curl/curl.h>

CROSS_BEGIN

class CurlHttpConnectorPrivate {
public:

    CurlHttpConnector *owner = nullptr;
    typedef CurlHttpConnectorPrivate private_type;

    void clear() {
        buffersz = 0;
        buffer.clear();
        rspheaders.clear();
    }

    static size_t ImpReceiveData(char const *buf, size_t size, size_t count, private_type
    *self) {
        size_t lbuf = size * count;
        if (!lbuf)
            return lbuf;

        self->buffer.write(buf, lbuf);

        CurlHttpConnector::memory_type mem(*self->buffer.rdbuf());
        mem.from = self->buffersz;
        mem.size = lbuf;
        self->buffersz += lbuf;

        self->owner->on_bytes(mem);

        return lbuf;
    }

    static int ImpProgress(private_type *self, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
        HttpConnector::progress_type down, up;

        down.from = 0;
        down.size = dltotal;
        down.value = dlnow;

        up.from = 0;
        up.size = ultotal;
        up.value = ulnow;

        self->owner->on_progress_download(down);
        self->owner->on_progress_upload(up);

        return CURLE_OK;
    }

    static size_t ImpReceiveHeader(char const *buf, size_t size, size_t count, private_type *self) {
        size_t lbuf = size * count;

        string bytes(buf, lbuf);
        size_t pos = bytes.find_first_of(':');
        if (pos == string::npos)
            return lbuf;
        string key(bytes.begin(), bytes.begin() + pos);
        string val(bytes.begin() + pos, bytes.end() - 2);
        self->rspheaders[key] = make_property(val);
        return lbuf;
    }

    CURL *h = nullptr;

    int errcode = -1;
    string errmsg;

    ::std::ostringstream buffer;
    size_t buffersz = -1;

    HttpConnector::args_type rspheaders;
    unsigned short respcode;
};

CurlHttpConnector::CurlHttpConnector() {
    NNT_CLASS_CONSTRUCT();
    d_ptr->owner = this;
}

CurlHttpConnector::~CurlHttpConnector() {
    NNT_CLASS_DESTORY();
}

string escape(CURL *h, HttpConnector::arg_type const &arg) {
    ::std::ostringstream oss;
    oss << arg;
    auto str = oss.str();
    char *res = curl_easy_escape(h, str.c_str(), str.length());
    if (res)
        str = res;
    return str;
}

string build_query(CURL *h, HttpConnector::args_type const &args) {
    ::std::vector<string> fs;
    for (auto &e : args) {
        ::std::ostringstream oss;
        oss << e.first << "=" << escape(h, e.second);
        fs.emplace_back(oss.str());
    }
    return implode(fs, "&");
}

void CurlHttpConnector::close() {
    if (d_ptr->h) {
        curl_easy_cleanup(d_ptr->h);
        d_ptr->h = nullptr;
    }

    d_ptr->clear();
}

bool CurlHttpConnector::send() const {
    NNT_THIS->close();

    string url = this->url;
    d_ptr->h = curl_easy_init();
    auto h = d_ptr->h;

    if (method == METHOD_GET) {
        if (!_reqargs.empty()) {
            if (url.find('?') == -1) {
                url += "/?";
            } else {
                url += "&";
            }
            url += build_query(h, _reqargs);
        }
    }

    curl_easy_setopt(h, CURLOPT_URL, url.c_str());
    curl_easy_setopt(h, CURLOPT_HEADER, full ? 1 : 0);
    curl_easy_setopt(h, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(h, CURLOPT_USERAGENT, ua.c_str());
    curl_easy_setopt(h, CURLOPT_CONNECTTIMEOUT, ctimeout);
    curl_easy_setopt(h, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(h, CURLOPT_NOSIGNAL, 1);

    // 不验证ssl
    curl_easy_setopt(h, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(h, CURLOPT_SSL_VERIFYPEER, 0);

    curl_httppost *form = nullptr;
    if ((method & METHOD_POST) == METHOD_POST) {
        curl_easy_setopt(h, CURLOPT_POST, 1);

        switch (method) {
            case METHOD_POST: {
                _reqheaders[HEADER_CONTENT_TYPE] = make_property("multipart/form-data");
                curl_httppost *last = nullptr;
                for (auto &e : _reqargs) {
                    auto val = escape(h, e.second);
                    curl_formadd(&form, &last,
                                 CURLFORM_COPYNAME, e.first.c_str(), CURLFORM_NAMELENGTH, e.first.length(),
                                 CURLFORM_COPYCONTENTS, val.c_str(), CURLFORM_CONTENTLEN, val.length(),
                                 CURLFORM_END);
                }
                curl_easy_setopt(h, CURLOPT_HTTPPOST, form);
            }
                break;
            case METHOD_POST_URLENCODED: {
                _reqheaders[HEADER_CONTENT_TYPE] = make_property("application/x-www-form-urlencoded; charset=utf-8;");
                auto val = build_query(h, _reqargs);
                curl_easy_setopt(h, CURLOPT_POSTFIELDSIZE, val.length());
                curl_easy_setopt(h, CURLOPT_COPYPOSTFIELDS, val.c_str());
            }
                break;
            case METHOD_POST_JSON: {
                _reqheaders[HEADER_CONTENT_TYPE] = make_property("application/json; charset=utf-8;");
                auto p = Combine(_reqargs);
                auto val = json_encode(*tojsonobj(*p));
                curl_easy_setopt(h, CURLOPT_POSTFIELDSIZE, val.length());
                curl_easy_setopt(h, CURLOPT_COPYPOSTFIELDS, val.c_str());
            }
                break;
            case METHOD_POST_XML: {
                _reqheaders[HEADER_CONTENT_TYPE] = make_property("application/xml; charset=utf-8;");
                auto p = Combine(_reqargs);
                auto val = xml_encode(*toxmlobj(*p));
                curl_easy_setopt(h, CURLOPT_POSTFIELDSIZE, val.length());
                curl_easy_setopt(h, CURLOPT_COPYPOSTFIELDS, val.c_str());
            }
                break;
        }
    }

    curl_slist *headers = nullptr;
    if (!_reqheaders.empty()) {
        for (auto &e : _reqheaders) {
            ::std::ostringstream oss;
            oss << e.first << ": " << e.second;
            auto val = oss.str();
            headers = curl_slist_append(headers, val.c_str());
        }
        curl_easy_setopt(h, CURLOPT_HTTPHEADER, headers);
    }

    curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, private_class_type::ImpReceiveData);
    curl_easy_setopt(h, CURLOPT_WRITEDATA, d_ptr);
    curl_easy_setopt(h, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(h, CURLOPT_XFERINFOFUNCTION, private_class_type::ImpProgress);
    curl_easy_setopt(h, CURLOPT_XFERINFODATA, d_ptr);
    curl_easy_setopt(h, CURLOPT_HEADERFUNCTION, private_class_type::ImpReceiveHeader);
    curl_easy_setopt(h, CURLOPT_HEADERDATA, d_ptr);

    on_connected();

    auto st = curl_easy_perform(h);
    if (st == CURLE_OK) {
        curl_easy_getinfo(h, CURLINFO_RESPONSE_CODE, &d_ptr->respcode);
        on_completed();
    } else {
        d_ptr->errcode = st;
        char const *msg = curl_easy_strerror(st);
        d_ptr->errmsg = msg;
        on_error(error(Code::FAILED, msg));
    }

    if (form)
        curl_formfree(form);
    if (headers)
        curl_slist_free_all(headers);
    curl_easy_cleanup(h);
    d_ptr->h = nullptr;

    on_disconnected();

    return RespondCodeIsOk(d_ptr->respcode);
}

int CurlHttpConnector::errcode() const {
    return d_ptr->errcode;
}

string const &CurlHttpConnector::errmsg() const {
    return d_ptr->errmsg;
}

Connector::stream_type const &CurlHttpConnector::body() const {
    return *d_ptr->buffer.rdbuf();
}

HttpConnector::args_type const &CurlHttpConnector::respheaders() const {
    return d_ptr->rspheaders;
}

unsigned short CurlHttpConnector::respcode() const {
    return d_ptr->respcode;
}

// --------------------------------------- DOWNLOAD CONNECTOR

class CurlDownloadConnectorPrivate {
public:

    CurlDownloadConnector *owner = nullptr;
    typedef CurlDownloadConnectorPrivate private_type;

    CurlDownloadConnectorPrivate() {
        buffer.proc_full = [&](FixedBuffer<BUFSIZ> &buf) {
            file->write(buf.buf(), buf.size());
        };
    }

    void clear() {
        buffer.clear();
        rspheaders.clear();
    }

    void flush() {
        if (buffer.size()) {
            file->write(buffer.buf(), buffer.size());
            buffer.clear();
        }
        file->flush();
    }

    static size_t ImpReceiveData(char const *buf, size_t size, size_t count, private_type
    *self) {
        size_t lbuf = size * count;
        if (!lbuf)
            return lbuf;

        self->buffer.write(buf, lbuf);

        ::std::stringbuf t(buf);
        CurlHttpConnector::memory_type mem(t);
        mem.from = 0;
        mem.size = lbuf;

        self->owner->on_bytes(mem);

        return lbuf;
    }

    static int ImpProgress(private_type *self, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
        HttpConnector::progress_type down, up;

        down.from = 0;
        down.size = dltotal;
        down.value = dlnow;

        up.from = 0;
        up.size = ultotal;
        up.value = ulnow;

        self->owner->on_progress_download(down);
        self->owner->on_progress_upload(up);

        return CURLE_OK;
    }

    static size_t ImpReceiveHeader(char const *buf, size_t size, size_t count, private_type *self) {
        size_t lbuf = size * count;

        string bytes(buf, lbuf);
        size_t pos = bytes.find_first_of(':');
        if (pos == string::npos)
            return lbuf;
        string key(bytes.begin(), bytes.begin() + pos);
        string val(bytes.begin() + pos, bytes.end() - 2);
        self->rspheaders[key] = make_property(val);
        return lbuf;
    }

    CURL *h = nullptr;

    int errcode = -1;
    string errmsg;

    // 下载缓存
    FixedBuffer<BUFSIZ> buffer;

    // 下载的文件指针
    shared_ptr<::std::ofstream> file;

    HttpConnector::args_type rspheaders;
    unsigned short respcode = -1;
};

CurlDownloadConnector::CurlDownloadConnector() {
    NNT_CLASS_CONSTRUCT();
    d_ptr->owner = this;
}

CurlDownloadConnector::~CurlDownloadConnector() {
    NNT_CLASS_DESTORY();
}

void CurlDownloadConnector::close() {
    if (d_ptr->h) {
        curl_easy_cleanup(d_ptr->h);
        d_ptr->h = nullptr;
    }

    d_ptr->clear();
}

bool CurlDownloadConnector::send() const {
    NNT_THIS->close();

    // 打开文件，进行读写
    d_ptr->file = make_shared<::std::ofstream>();
    d_ptr->file->open(target, ::std::ios::out | ::std::ios::binary);

    // 开始连接服务器
    string url = this->url;
    d_ptr->h = curl_easy_init();
    auto h = d_ptr->h;

    if (method == METHOD_GET) {
        if (!_reqargs.empty()) {
            if (url.find('?') == -1) {
                url += "/?";
            } else {
                url += "&";
            }
            url += build_query(h, _reqargs);
        }
    }

    curl_easy_setopt(h, CURLOPT_URL, url.c_str());
    curl_easy_setopt(h, CURLOPT_HEADER, full ? 1 : 0);
    curl_easy_setopt(h, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(h, CURLOPT_USERAGENT, ua.c_str());
    curl_easy_setopt(h, CURLOPT_CONNECTTIMEOUT, ctimeout);
    curl_easy_setopt(h, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(h, CURLOPT_NOSIGNAL, 1);

    // 不验证ssl
    curl_easy_setopt(h, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(h, CURLOPT_SSL_VERIFYPEER, 0);

    curl_httppost *form = nullptr;
    if ((method & METHOD_POST) == METHOD_POST) {
        curl_easy_setopt(h, CURLOPT_POST, 1);

        switch (method) {
            case METHOD_POST: {
                _reqheaders[HEADER_CONTENT_TYPE] = make_property("multipart/form-data");
                curl_httppost *last = nullptr;
                for (auto &e : _reqargs) {
                    auto val = escape(h, e.second);
                    curl_formadd(&form, &last,
                                 CURLFORM_COPYNAME, e.first.c_str(), CURLFORM_NAMELENGTH, e.first.length(),
                                 CURLFORM_COPYCONTENTS, val.c_str(), CURLFORM_CONTENTLEN, val.length(),
                                 CURLFORM_END);
                }
                curl_easy_setopt(h, CURLOPT_HTTPPOST, form);
            }
                break;
            case METHOD_POST_URLENCODED: {
                _reqheaders[HEADER_CONTENT_TYPE] = make_property("application/x-www-form-urlencoded; charset=utf-8;");
                auto val = build_query(h, _reqargs);
                curl_easy_setopt(h, CURLOPT_POSTFIELDSIZE, val.length());
                curl_easy_setopt(h, CURLOPT_COPYPOSTFIELDS, val.c_str());
            }
                break;
            case METHOD_POST_JSON: {
                _reqheaders[HEADER_CONTENT_TYPE] = make_property("application/json; charset=utf-8;");
                auto p = Combine(_reqargs);
                auto val = json_encode(*tojsonobj(*p));
                curl_easy_setopt(h, CURLOPT_POSTFIELDSIZE, val.length());
                curl_easy_setopt(h, CURLOPT_COPYPOSTFIELDS, val.c_str());
            }
                break;
            case METHOD_POST_XML: {
                _reqheaders[HEADER_CONTENT_TYPE] = make_property("application/xml; charset=utf-8;");
                auto p = Combine(_reqargs);
                auto val = xml_encode(*toxmlobj(*p));
                curl_easy_setopt(h, CURLOPT_POSTFIELDSIZE, val.length());
                curl_easy_setopt(h, CURLOPT_COPYPOSTFIELDS, val.c_str());
            }
                break;
        }
    }

    curl_slist *headers = nullptr;
    if (!_reqheaders.empty()) {
        for (auto &e : _reqheaders) {
            ::std::ostringstream oss;
            oss << e.first << ": " << e.second;
            auto val = oss.str();
            headers = curl_slist_append(headers, val.c_str());
        }
        curl_easy_setopt(h, CURLOPT_HTTPHEADER, headers);
    }

    curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, private_class_type::ImpReceiveData);
    curl_easy_setopt(h, CURLOPT_WRITEDATA, d_ptr);
    curl_easy_setopt(h, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(h, CURLOPT_XFERINFOFUNCTION, private_class_type::ImpProgress);
    curl_easy_setopt(h, CURLOPT_XFERINFODATA, d_ptr);
    curl_easy_setopt(h, CURLOPT_HEADERFUNCTION, private_class_type::ImpReceiveHeader);
    curl_easy_setopt(h, CURLOPT_HEADERDATA, d_ptr);

    auto st = curl_easy_perform(h);
    if (st == CURLE_OK) {
        curl_easy_getinfo(h, CURLINFO_RESPONSE_CODE, &d_ptr->respcode);
        // 刷缓存中的数据
        d_ptr->flush();
        on_completed();
    } else {
        d_ptr->errcode = st;
        char const *msg = curl_easy_strerror(st);
        d_ptr->errmsg = msg;
        on_error(error(Code::FAILED, msg));
    }

    if (form)
        curl_formfree(form);
    if (headers)
        curl_slist_free_all(headers);
    curl_easy_cleanup(h);
    d_ptr->h = nullptr;

    return RespondCodeIsOk(d_ptr->respcode);
}

int CurlDownloadConnector::errcode() const {
    return d_ptr->errcode;
}

string const &CurlDownloadConnector::errmsg() const {
    return d_ptr->errmsg;
}

Connector::stream_type const &CurlDownloadConnector::body() const {
    return *d_ptr->file->rdbuf();
}

HttpConnector::args_type const &CurlDownloadConnector::respheaders() const {
    return d_ptr->rspheaders;
}

unsigned short CurlDownloadConnector::respcode() const {
    return d_ptr->respcode;
}

CROSS_END
