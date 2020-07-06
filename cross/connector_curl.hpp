#ifndef __NNTCROSS_CONNECTOR_CURL_H_INCLUDED
#define __NNTCROSS_CONNECTOR_CURL_H_INCLUDED

#include "connector.hpp"

CROSS_BEGIN

NNT_CLASS_PREPARE(CurlHttpConnector);

class NNT_API CurlHttpConnector : public HttpConnector
{
NNT_CLASS_DECL(CurlHttpConnector);

public:

	CurlHttpConnector();

	virtual ~CurlHttpConnector();

	virtual void close() override;

	virtual bool send() const override;

	virtual int errcode() const override;

	virtual string const& errmsg() const override;

	virtual body_stream_type body() const override;

	virtual args_type const& respheaders() const override;

	virtual unsigned short respcode() const override;

};

NNT_CLASS_PREPARE(CurlDownloadConnector);

class NNT_API CurlDownloadConnector : public DownloadConnector
{
NNT_CLASS_DECL(CurlDownloadConnector);

public:

	CurlDownloadConnector();

	virtual ~CurlDownloadConnector();

	virtual void close() override;

	virtual bool send() const override;

	virtual int errcode() const override;

	virtual string const& errmsg() const override;

	virtual args_type const& respheaders() const override;

	virtual unsigned short respcode() const override;
};

CROSS_END

#endif
