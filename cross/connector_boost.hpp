#ifndef __CROSS_CONNECTOR_BOOST_H_INCLUDED
#define __CROSS_CONNECTOR_BOOST_H_INCLUDED

#include "connector.hpp"

CROSS_BEGIN

NNT_CLASS_PREPARE(BoostWebSocketConnector);

class NNT_API BoostWebSocketConnector
    : public WebSocketConnector 
{
    NNT_CLASS_DECL(BoostWebSocketConnector);

public:

    BoostWebSocketConnector();
    virtual ~BoostWebSocketConnector();

    virtual bool connect() override;

    virtual void close() override;

    virtual buffer_typep wait() override;

protected:

    virtual bool _write(memory_type const &) override;

};

CROSS_END

#endif