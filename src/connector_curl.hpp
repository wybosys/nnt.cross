#ifndef __NNTCROSS_CONNECTOR_CURL_H_INCLUDED
#define __NNTCROSS_CONNECTOR_CURL_H_INCLUDED

#include "connector.hpp"

CROSS_BEGIN

NNT_CLASS_PREPARE(CurlHttpConnector);

class NNT_API CurlHttpConnector : public HttpConnector {
NNT_CLASS_DECL(CurlHttpConnector);

public:

    CurlHttpConnector();

    virtual ~CurlHttpConnector();

    virtual void close();

    virtual bool send() const;

    virtual int errcode() const;

    virtual string const &errmsg() const;

    virtual body_stream_type body() const;

    virtual args_type const &respheaders() const;

    virtual unsigned short respcode() const;

};

NNT_CLASS_PREPARE(CurlDownloadConnector);

class NNT_API CurlDownloadConnector : public DownloadConnector {
NNT_CLASS_DECL(CurlDownloadConnector);

public:

    CurlDownloadConnector();

    virtual ~CurlDownloadConnector();

    virtual void close();

    virtual bool send() const;

    virtual int errcode() const;

    virtual string const &errmsg() const;

    virtual args_type const &respheaders() const;

    virtual unsigned short respcode() const;
};

CROSS_END

#endif
