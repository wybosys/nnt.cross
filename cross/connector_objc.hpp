#ifndef __CROSS_CONNECTOR_OBJC_H_INCLUDED
#define __CROSS_CONNECTOR_OBJC_H_INCLUDED

#include "connector.hpp"

CROSS_BEGIN

NNT_CLASS_PREPARE(ObjcHttpConnector);

class NNT_API ObjcHttpConnector
: public HttpConnector
{
    NNT_CLASS_DECL(ObjcHttpConnector);
    
public:
    
    ObjcHttpConnector();
    virtual ~ObjcHttpConnector();
    
    virtual void close() override;
    
    virtual bool send() const override;
    
    virtual int errcode() const override;
    
    virtual string const& errmsg() const override;
    
    virtual body_stream_type body() const override;
    
    virtual args_type const& respheaders() const override;
    
    virtual unsigned short respcode() const override;
};

NNT_CLASS_PREPARE(ObjcDownloadConnector);

class NNT_API ObjcDownloadConnector
: public DownloadConnector
{
    NNT_CLASS_DECL(ObjcDownloadConnector);
    
public:
    
    ObjcDownloadConnector();
    virtual ~ObjcDownloadConnector();
    
    virtual void close() override;
    
    virtual bool send() const override;
    
    virtual int errcode() const override;
    
    virtual string const& errmsg() const override;
    
    virtual args_type const& respheaders() const override;
    
    virtual unsigned short respcode() const override;
};

CROSS_END

#endif
