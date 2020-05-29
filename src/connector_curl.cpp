#include "cross.h"
#include "connector_curl.h"
#include "str.h"
#include <curl/curl.h>
#include <sstream>
#include "json.h"
#include "xml.h"

CROSS_BEGIN

class CurlHttpConnectorPrivate
{
public:

    CurlHttpConnector *owner;
    typedef CurlHttpConnectorPrivate private_type;

    void clear()
    {
        buffer.clear();
        rspheaders.clear();
    }

    static size_t ImpReceiveData(char const* buf, size_t size, size_t count, private_type
        * self)
    {
        size_t lbuf = size * count;
        self->buffer.write(buf, lbuf);
        return lbuf;
    }

    static int ImpProgress(private_type* self, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
    {
        HttpConnector::progress_type down, up;

        down.from = 0;
        down.to = dltotal;
        down.value = dlnow;

        up.from = 0;
        up.to = ultotal;
        up.value = ulnow;

        self->owner->on_progress_download(down);
        self->owner->on_progress_upload(up);

        return CURLE_OK;
    }

    static size_t ImpReceiveHeader(char const* buf, size_t size, size_t count, private_type *self)
    {
        size_t lbuf = size * count;

        string bytes(buf, lbuf);
        size_t pos = bytes.find_first_of(":");
        if (pos == string::npos)
            return lbuf;
        string key(bytes.begin(), bytes.begin() + pos);
        string val(bytes.begin() + pos, bytes.end() - 2);
        self->rspheaders[key] = _P(val);
        return lbuf;
    }
    
    int errcode;
    string errmsg;
    ostringstream buffer;
    HttpConnector::args_type rspheaders;

    unsigned short respcode;
};

CurlHttpConnector::CurlHttpConnector()
{
    NNT_CLASS_CONSTRUCT();
    d_ptr->owner = this;
}

CurlHttpConnector::~CurlHttpConnector()
{
    NNT_CLASS_DESTORY();
}

string escape(CURL* h, HttpConnector::arg_type const& arg)
{
    ostringstream oss;
    oss << arg;
    auto str = oss.str();
    char* res = curl_easy_escape(h, str.c_str(), str.length());
    // 不需要释放res
    if (res)
        str = res;
    return str;
}

string build_query(CURL* h, HttpConnector::args_type const& args)
{
    vector<string> fs;
    for (auto& e : args) {
        ostringstream oss;
        oss << e.first << "=" << escape(h, e.second);
        fs.emplace_back(oss.str());
    }
    return implode(fs, "&");
}

void CurlHttpConnector::close()
{
    d_ptr->clear();
}

bool CurlHttpConnector::send() const {
    NNT_THIS->close();

    string url = this->url;
    auto h = curl_easy_init();

    // 处理get请求
    if (method == METHOD_GET) {
        if (!_reqargs.empty()) {
            if (url.find('?') == -1) {
                url += "/?";
            }
            else {
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

    // 跳过https证书验证
    curl_easy_setopt(h, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(h, CURLOPT_SSL_VERIFYPEER, 0);

    // 处理post请求
    curl_httppost *form = nullptr;
    if ((method & METHOD_POST) == METHOD_POST) {
        curl_easy_setopt(h, CURLOPT_POST, 1);

        switch (method) {
        case METHOD_POST: {
            _reqheaders[HEADER_CONTENT_TYPE] = _P("multipart/form-data");
            curl_httppost *last = nullptr;
            for (auto &e : _reqargs) {
                auto val = escape(h, e.second);
                curl_formadd(&form, &last,
                    CURLFORM_COPYNAME, e.first.c_str(), CURLFORM_NAMELENGTH, e.first.length(),
                    CURLFORM_COPYCONTENTS, val.c_str(), CURLFORM_CONTENTLEN, val.length(),
                    CURLFORM_END);
            }
            curl_easy_setopt(h, CURLOPT_HTTPPOST, form);
        } break;
        case METHOD_POST_URLENCODED: {
            _reqheaders[HEADER_CONTENT_TYPE] = _P("application/x-www-form-urlencoded; charset=utf-8;");
            auto val = build_query(h, _reqargs);
            curl_easy_setopt(h, CURLOPT_POSTFIELDSIZE, val.length());
            curl_easy_setopt(h, CURLOPT_COPYPOSTFIELDS, val.c_str());
        } break;
        case METHOD_POST_JSON: {
            _reqheaders[HEADER_CONTENT_TYPE] = _P("application/json; charset=utf-8;");
            auto p = Combine(_reqargs);
            auto val = json_encode(*tojsonobj(*p));
            curl_easy_setopt(h, CURLOPT_POSTFIELDSIZE, val.length());
            curl_easy_setopt(h, CURLOPT_COPYPOSTFIELDS, val.c_str());
        } break;
        case METHOD_POST_XML: {
            _reqheaders[HEADER_CONTENT_TYPE] = _P("application/xml; charset=utf-8;");
            auto p = Combine(_reqargs);
            auto val = xml_encode(*toxmlobj(*p));
            curl_easy_setopt(h, CURLOPT_POSTFIELDSIZE, val.length());
            curl_easy_setopt(h, CURLOPT_COPYPOSTFIELDS, val.c_str());
        } break;
        }
    }

    // 设置请求头
    curl_slist *headers = nullptr;
    if (!_reqheaders.empty()) {
        for (auto &e : _reqheaders) {
            ostringstream oss;
            oss << e.first << ": " << e.second;
            auto val = oss.str();
            headers = curl_slist_append(headers, val.c_str());
        }
        curl_easy_setopt(h, CURLOPT_HTTPHEADER, headers);
    }

    // 跳过https证书验证
    curl_easy_setopt(h, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(h, CURLOPT_SSL_VERIFYPEER, 0);

    // 设置返回的处理
    curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, private_class_type::ImpReceiveData);
    curl_easy_setopt(h, CURLOPT_WRITEDATA, d_ptr);
    curl_easy_setopt(h, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(h, CURLOPT_XFERINFOFUNCTION, private_class_type::ImpProgress);
    curl_easy_setopt(h, CURLOPT_XFERINFODATA, d_ptr);
    curl_easy_setopt(h, CURLOPT_HEADERFUNCTION, private_class_type::ImpReceiveHeader);
    curl_easy_setopt(h, CURLOPT_HEADERDATA, d_ptr);

    // 发起请求
    const int st = curl_easy_perform(h);
    bool suc = false;
    if (st == CURLE_OK) {
        curl_easy_getinfo(h, CURLINFO_RESPONSE_CODE, &d_ptr->respcode);
        // 全部数据读取完成回调
        on_bytes();
        suc = true;
    }
    else {
        d_ptr->errcode = st;
        d_ptr->errmsg = "curl_easy_perform 执行失败";
    }

    // 清理
    if (form)
        curl_formfree(form);
    if (headers)
        curl_slist_free_all(headers);

    curl_easy_cleanup(h);
    return suc;
}

int CurlHttpConnector::errcode() const {
    return d_ptr->errcode;
}

string const& CurlHttpConnector::errmsg() const {
    return d_ptr->errmsg;
}

stringbuf const& CurlHttpConnector::body() const {
    return *d_ptr->buffer.rdbuf();
}

HttpConnector::args_type const& CurlHttpConnector::respheaders() const {
    return d_ptr->rspheaders;
}

unsigned short CurlHttpConnector::respcode() const {
    return d_ptr->respcode;
}

CROSS_END