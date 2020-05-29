#ifndef __NNTCROSS_CONNECTOR_H_INCLUDED
#define __NNTCROSS_CONNECTOR_H_INCLUDED

#include "code.h"
#include "property.h"
#include <map>

CROSS_BEGIN

class NNT_API Connector : public Object
{
public:

    // 关闭连接
    virtual void close();

    // 访问地址
    string url;

    // 端口
    unsigned short port;

    // connection time out
    static unsigned int CTIMEOUT;

    // timeout
    static unsigned int TIMEOUT;

    // ua
    static string USERAGENT;

    typedef Range<size_t> range_type;
    typedef Progress<size_t> progress_type;

protected:

    virtual void on_connected() {} // 连接成功
    virtual void on_progress(range_type const&) {} // 传输进度
    virtual void on_bytes(range_type const&) {} // 收到完整数据
    virtual void on_error(error const&) {} // 遇到错误
    virtual void on_disconnect() {} // 断开连接

};

class NNT_API HttpConnector : public Connector
{
public:

    // 请求形式
    enum {
        METHOD_GET = 0x1000,
        METHOD_POST = 0x1000000,
        METHOD_POST_URLENCODED = METHOD_POST | 1,
        METHOD_POST_XML = METHOD_POST | 2,
        METHOD_POST_JSON = METHOD_POST | 3,
    };

    unsigned int method = METHOD_GET;

    typedef Property arg_type;
    typedef map<string, arg_type> args_type;
    typedef map<string, string> files_type;

    // 是否完整获取返回数据
    bool full = false;

    // 代理
    string ua = USERAGENT;

    // 超时s
    unsigned int ctimeout = CTIMEOUT; // connection time out
    unsigned int timeout = TIMEOUT; // timeout

        // 设置参数
    virtual HttpConnector& setargs(args_type const&);

    // 获得参数值
    virtual arg_type const& getarg(string const&);

    // 是否存在参数
    virtual bool hasarg(string const&);

    // 设置请求头
    virtual HttpConnector& setheaders(args_type const&);

    // 读取请求头
    virtual arg_type const& getheader(string const&);

    // 是否存在请求头
    virtual bool hasheader(string const&);

    // 上传文件
    virtual bool uploads(files_type const&);

    // 执行请求
    virtual string const& send() const;

    // 如果请求错误，保存错误信息
    virtual int errcode();
    virtual string const& errmsg();

    // 返回的消息主体
    virtual string const& body();

    // 返回的头
    virtual args_type const& respheaders();

protected:

    mutable args_type _reqheaders;
    args_type _rspheaders, _reqargs;
};

class NNT_API WebSocketConnector : public Connector
{

};

CROSS_END

#endif