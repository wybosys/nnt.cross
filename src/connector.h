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
    virtual void close();

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

    typedef Range<size_t> range_type;
    typedef Progress<size_t> progress_type;

protected:

    virtual void on_connected() {} // ���ӳɹ�
    virtual void on_progress(range_type const&) {} // �������
    virtual void on_bytes(range_type const&) {} // �յ���������
    virtual void on_error(error const&) {} // ��������
    virtual void on_disconnect() {} // �Ͽ�����

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

    typedef Property arg_type;
    typedef map<string, arg_type> args_type;
    typedef map<string, string> files_type;

    // �Ƿ�������ȡ��������
    bool full = false;

    // ����
    string ua = USERAGENT;

    // ��ʱs
    unsigned int ctimeout = CTIMEOUT; // connection time out
    unsigned int timeout = TIMEOUT; // timeout

        // ���ò���
    virtual HttpConnector& setargs(args_type const&);

    // ��ò���ֵ
    virtual arg_type const& getarg(string const&);

    // �Ƿ���ڲ���
    virtual bool hasarg(string const&);

    // ��������ͷ
    virtual HttpConnector& setheaders(args_type const&);

    // ��ȡ����ͷ
    virtual arg_type const& getheader(string const&);

    // �Ƿ��������ͷ
    virtual bool hasheader(string const&);

    // �ϴ��ļ�
    virtual bool uploads(files_type const&);

    // ִ������
    virtual string const& send() const;

    // ���������󣬱��������Ϣ
    virtual int errcode();
    virtual string const& errmsg();

    // ���ص���Ϣ����
    virtual string const& body();

    // ���ص�ͷ
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