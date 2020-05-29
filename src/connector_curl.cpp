#include "cross.h"
#include "connector_curl.h"

CROSS_BEGIN

class CurlHttpConnectorPrivate
{
public:
};

CurlHttpConnector::CurlHttpConnector()
{
    NNT_CLASS_CONSTRUCT();
}

CurlHttpConnector::~CurlHttpConnector()
{
    NNT_CLASS_DESTORY();
}

CROSS_END