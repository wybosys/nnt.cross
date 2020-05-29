#ifndef __NNTCROSS_CONNECTOR_CURL_H_INCLUDED
#define __NNTCROSS_CONNECTOR_CURL_H_INCLUDED

#include "connector.h"

CROSS_BEGIN

NNT_CLASS_PREPARE(CurlHttpConnector);

class NNT_API CurlHttpConnector : public HttpConnector
{
    NNT_CLASS_DECL(CurlHttpConnector);

public:

    CurlHttpConnector();
    virtual ~CurlHttpConnector();

};

CROSS_END

#endif