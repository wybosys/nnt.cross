#ifndef __NNTCROSS_CONNECTOR_H_INCLUDED
#define __NNTCROSS_CONNECTOR_H_INCLUDED

#include "code.hpp"
#include "property.hpp"
#include <map>

CROSS_BEGIN

class NNT_API Connector : virtual public Object {
public:

    // 关闭当前链接
    virtual void close() = 0;

    // 地址
    string url;

    // 全局建立连接超时
    static unsigned int CTIMEOUT;

    // 全局数据超时
    static unsigned int TIMEOUT;

    // 全局客户端标记
    static string USERAGENT;

    typedef Progress<unsigned long long> progress_type;
    typedef ::std::streambuf stream_type;
    typedef Memory<::std::stringbuf &, size_t> memory_type;
    typedef shared_ptr<Property> arg_type;
    typedef ::std::map<string, arg_type> args_type;
    typedef ::std::map<string, string> files_type;

protected:

    virtual void on_connected() const {} // 连接成功
    virtual void on_progress(progress_type const &) const {} // 传输进度
    virtual void on_bytes(memory_type const &) const {} // 收到部分数据
    virtual void on_completed() const {} // 完成数据传输
    virtual void on_error(error const &) const {} // 遇到错误
    virtual void on_disconnected() const {} // 断开连接

};

class NNT_API HttpConnector : virtual public Connector {
public:

    typedef unsigned short respondcode_type;

    // 请求形式
    enum {
        METHOD_GET = 0x1000,
        METHOD_POST = 0x1000000,
        METHOD_POST_URLENCODED = METHOD_POST | 1,
        METHOD_POST_XML = METHOD_POST | 2,
        METHOD_POST_JSON = METHOD_POST | 3,
    };

    unsigned int method = METHOD_GET;

    const string HEADER_CONTENT_TYPE = "Content-Type";

    // 是否完整获取返回数据
    bool full = false;

    // 代理
    string ua = USERAGENT;

    // 超时s
    unsigned int ctimeout = CTIMEOUT; // connection time out
    unsigned int timeout = TIMEOUT; // timeout

    // 设置参数
    virtual HttpConnector &setarg(string const &, arg_type const &);

    virtual HttpConnector &setargs(args_type const &);

    template<typename T>
    inline HttpConnector &setarg(string const &key, T const &v) {
        return setarg(key, make_shared<Property>(v));
    }

    // 获得参数值
    virtual arg_type const &getarg(string const &);

    // 是否存在参数
    virtual bool hasarg(string const &);

    // 设置请求头
    virtual HttpConnector &setheader(string const &, arg_type const &);

    virtual HttpConnector &setheaders(args_type const &);

    template<typename T>
    inline HttpConnector &setheader(string const &key, T const &v) {
        return setheader(key, make_shared<Property>(v));
    }

    // 读取请求头
    virtual arg_type const &getheader(string const &);

    // 是否存在请求头
    virtual bool hasheader(string const &);

    // 执行请求
    virtual bool send() const = 0;

    // 如果请求错误，保存错误信息
    virtual int errcode() const = 0;

    virtual string const &errmsg() const = 0;

    // 返回的消息主体
    virtual stream_type const &body() const = 0;

    // 返回的头
    virtual args_type const &respheaders() const = 0;

    // 返回的错误码
    virtual respondcode_type respcode() const = 0;

    // 根据错误码计算请求是否成功
    static bool RespondCodeIsOk(respondcode_type);

protected:

    // 分开上传下载进度回调，默认on_progress为下载进度回调
    virtual void on_progress_upload(progress_type const &) const {}

    virtual void on_progress_download(progress_type const &range) const {
        on_progress(range);
    }

    mutable args_type _reqheaders;
    args_type _reqargs;
};

// websocket连接器
class NNT_API WebSocketConnector : virtual public Connector {
public:

    // 连接服务器
    virtual bool connect() = 0;

    // 发送数据
    virtual bool write(memory_type const &) = 0;

    // 等待数据
    virtual void wait() = 0;

    // 自动重试的最大次数，-1代表持续重试
    int maxretrys = MAXRETRYS;

    static int MAXRETRYS;

protected:

    // 尝试重连服务器
    virtual void reconnect() {
        connect();
    }

    // 正在连接
    virtual void on_connecting() const {} // 正在连接
    virtual void on_reconnecting() const {} // 正在重连
    virtual void on_reconnected() const {} // 已经重连上
};

// 下载连接器
class NNT_API DownloadConnector : virtual public HttpConnector {
public:

    // 下载后保存的文件位置
    string target;

    // 是否支持断点续传
    bool resumable = false;
};

// 转换args到property，之后既可以使用property的序列化方法
extern Connector::arg_type Combine(Connector::args_type const &);

CROSS_END

#endif
