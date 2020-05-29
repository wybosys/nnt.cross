#ifndef __NNTCROSS_CONNECTOR_H_INCLUDED
#define __NNTCROSS_CONNECTOR_H_INCLUDED

#include "code.h"
#include "property.h"
#include <map>

CROSS_BEGIN

class NNT_API Connector : public Object
{
public:

    // �ر�����
    virtual void close() {}

    // ���ʵ�ַ
    string url;

    // �˿�
    unsigned short port;

    // connection time out
    static unsigned int CTIMEOUT;

    // timeout
    static unsigned int TIMEOUT;

    // ua
    static string USERAGENT;

    typedef Progress<unsigned long long> progress_type;

protected:

    virtual void on_connected() const {} // ���ӳɹ�
    virtual void on_progress(progress_type const&) const {} // �������
    virtual void on_bytes() const {} // �յ���������
    virtual void on_error(error const&) const {} // ��������
    virtual void on_disconnect() const {} // �Ͽ�����

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

    typedef shared_ptr<Property> arg_type;
    typedef map<string, arg_type> args_type;
    typedef map<string, string> files_type;

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

    // �ϴ��ļ�
    virtual bool uploads(files_type const&);

    // ִ������
    virtual bool send() const;

    // ���������󣬱��������Ϣ
    virtual int errcode() const;
    virtual string const& errmsg() const;

    // ���ص���Ϣ����
    virtual stringbuf const& body() const;

    // ���ص�ͷ
    virtual args_type const& respheaders() const;

    // ���صĴ�����
    virtual unsigned short respcode() const;

protected:

    // �ֿ��ϴ����ؽ��Ȼص���Ĭ��on_progressΪ���ؽ��Ȼص�
    virtual void on_progress_upload(progress_type const&) const {}
    virtual void on_progress_download(progress_type const& range) const {
        on_progress(range);
    }

    mutable args_type _reqheaders;
    args_type _reqargs;
};

class NNT_API WebSocketConnector : public Connector
{

};

CROSS_END

#endif