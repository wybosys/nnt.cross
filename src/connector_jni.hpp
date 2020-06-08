#ifndef __NNTCROSS_CONNECTOR_JNI_H_INCLUDED
#define __NNTCROSS_CONNECTOR_JNI_H_INCLUDED

#include "connector.hpp"

CROSS_BEGIN

NNT_CLASS_PREPARE(JniHttpConnector);

class NNT_API JniHttpConnector : public HttpConnector
{
    NNT_CLASS_DECL(JniHttpConnector);

public:

    JniHttpConnector(string const& JavaClassPath);
    virtual ~JniHttpConnector();

    virtual void close();
    virtual bool send() const;

    virtual int errcode() const;
    virtual string const& errmsg() const;
    virtual body_stream_type body() const;
    virtual args_type const& respheaders() const;
    virtual unsigned short respcode() const;
};

NNT_CLASS_PREPARE(JniWebSocketConnector);

class NNT_API JniWebSocketConnector : public WebSocketConnector
{
    NNT_CLASS_DECL(JniWebSocketConnector);

public:

    JniWebSocketConnector(string const& JavaClassPath);
    virtual ~JniWebSocketConnector();

    virtual void close();
    virtual bool connect();
    virtual bool write(memory_type const&);
    virtual buffer_type wait();
};

CROSS_END

#endif