#ifndef __NNTCROSS_CONNECTOR_LWS_H_INCLUDED
#define __NNTCROSS_CONNECTOR_LWS_H_INCLUDED

#include "connector.hpp"

CROSS_BEGIN

NNT_CLASS_PREPARE(LibWebSocketConnector);

class NNT_API LibWebSocketConnector : public WebSocketConnector {
NNT_CLASS_DECL(LibWebSocketConnector);

public:

    LibWebSocketConnector();

    virtual ~LibWebSocketConnector();

    virtual bool connect();

    virtual void close();

    virtual bool write(memory_type const &);

private:

    virtual void wait();

};

CROSS_END

#endif
