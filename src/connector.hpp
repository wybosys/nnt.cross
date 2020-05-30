#ifndef __NNTCROSS_CONNECTOR_H_INCLUDED
#define __NNTCROSS_CONNECTOR_H_INCLUDED

#include "code.hpp"
#include "property.hpp"
#include <map>

CROSS_BEGIN

class NNT_API Connector : public Object
{
public:

    // �ر�����
    virtual void close() = 0;

    // ���ʵ�ַ
    string url;

    // connection time out
    static unsigned int CTIMEOUT;

    // timeout
    static unsigned int TIMEOUT;

    // ua
    static string USERAGENT;

    typedef Progress<unsigned long long> progress_type;
    typedef Memory<stringbuf&, size_t> memory_type;
    typedef shared_ptr<Property> arg_type;
    typedef map<string, arg_type> args_type;
    typedef map<string, string> files_type;

protected:

    virtual void on_connected() const {} // ���ӳɹ�
    virtual void on_progress(progress_type const&) const {} // �������
    virtual void on_bytes(memory_type const&) const {} // �յ���������
    virtual void on_completed() const {} // ������ݴ���
    virtual void on_error(error const&) const {} // ��������
    virtual void on_disconnected() const {} // �Ͽ�����

};

class NNT_API HttpConnector : public Connector
{
public:

    // ������ʽ
    enum {
        METHOD_GET = 0x1000,
        METHOD_POST = 0x1000000,
        METHOD_POST_URLENCODED = METHOD_POST | 1,
        METHOD_POST_XML = METHOD_POST | 2,
        METHOD_POST_JSON = METHOD_POST | 3,
    };

    unsigned int method = METHOD_GET;

    const string HEADER_CONTENT_TYPE = "Content-Type";

    // �Ƿ�������ȡ��������
    bool full = false;

    // ����
    string ua = USERAGENT;

    // ��ʱs
    unsigned int ctimeout = CTIMEOUT; // connection time out
    unsigned int timeout = TIMEOUT; // timeout

    // ���ò���
    virtual HttpConnector& setarg(string const&, arg_type const&);
    virtual HttpConnector& setargs(args_type const&);

    template <typename T>
    inline HttpConnector& setarg(string const& key, T const& v) {
        return setarg(key, make_shared<Property>(v));
    }

    // ��ò���ֵ
    virtual arg_type const& getarg(string const&);

    // �Ƿ���ڲ���
    virtual bool hasarg(string const&);

    // ��������ͷ
    virtual HttpConnector& setheader(string const&, arg_type const&);
    virtual HttpConnector& setheaders(args_type const&);

    template <typename T>
    inline HttpConnector& setheader(string const& key, T const& v) {
        return setheader(key, make_shared<Property>(v));
    }

    // ��ȡ����ͷ
    virtual arg_type const& getheader(string const&);

    // �Ƿ��������ͷ
    virtual bool hasheader(string const&);

    // ִ������
    virtual bool send() const = 0;

    // ���������󣬱��������Ϣ
    virtual int errcode() const = 0;
    virtual string const& errmsg() const = 0;

    // ���ص���Ϣ����
    virtual stringbuf const& body() const = 0;

    // ���ص�ͷ
    virtual args_type const& respheaders() const = 0;

    // ���صĴ�����
    virtual unsigned short respcode() const = 0;

protected:

    // �ֿ��ϴ����ؽ��Ȼص���Ĭ��on_progressΪ���ؽ��Ȼص�
    virtual void on_progress_upload(progress_type const&) const {}
    virtual void on_progress_download(progress_type const& range) const {
        on_progress(range);
    }

    mutable args_type _reqheaders;
    args_type _reqargs;
};

// websocket������
class NNT_API WebSocketConnector : public Connector
{
public:

    // ���ӷ�����
    virtual bool connect() = 0;

    // ��������
    virtual bool write(memory_type const&) = 0;

    // �ȴ�����
    virtual void wait() = 0;

    // �Զ����Ե���������-1�����������
    int maxretrys = MAXRETRYS;

    static int MAXRETRYS;

protected:

    // ����������
    virtual void reconnect() {
        connect();
    }

    // ��������
    virtual void on_connecting() const {}
    virtual void on_reconnecting() const {}
    virtual void on_reconnected() const {}
};

// ת��args��property��֮��ȿ���ʹ��property�����л�����
extern Connector::arg_type Combine(Connector::args_type const&);

CROSS_END

#endif