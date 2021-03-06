#ifndef __NNTCROSS_CONNECTOR_LWS_H_INCLUDED
#define __NNTCROSS_CONNECTOR_LWS_H_INCLUDED

#include "connector.hpp"

CROSS_BEGIN

NNT_CLASS_PREPARE(LibWebSocketConnector);

class NNT_API LibWebSocketConnector 
    : public WebSocketConnector 
{
    NNT_CLASS_DECL(LibWebSocketConnector);

public:

    LibWebSocketConnector();

    virtual ~LibWebSocketConnector();

    virtual bool connect() override;
    
    virtual void close() override;

    virtual buffer_typep wait() override;

protected:

    virtual bool _write(memory_type const &) override;

};

CROSS_END

#endif
