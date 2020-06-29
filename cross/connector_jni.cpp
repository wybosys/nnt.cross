#include "cross.hpp"
#include "connector_jni.hpp"
#include <ajni++/ajni++.hpp>
#include <sstream>

CROSS_BEGIN

class JniHttpConnectorPrivate
{
public:

};

JniHttpConnector::JniHttpConnector(string const& JavaClassPath)
{
    NNT_CLASS_CONSTRUCT();
}

JniHttpConnector::~JniHttpConnector()
{
    NNT_CLASS_DESTORY();
}

void JniHttpConnector::close()
{

}

bool JniHttpConnector::send() const
{
    return false;
}

int JniHttpConnector::errcode() const
{
    return 0;
}

string const& JniHttpConnector::errmsg() const
{
    return ::NNT_NS::Nil<string>();
}

HttpConnector::body_stream_type JniHttpConnector::body() const
{
    return ::NNT_NS::Nil<stream_type>();
}

JniHttpConnector::args_type const& JniHttpConnector::respheaders() const
{
    return ::NNT_NS::Nil<args_type>();
}

unsigned short JniHttpConnector::respcode() const
{
    return 0;
}

class JniWebSocketConnectorPrivate
{
public:

};

JniWebSocketConnector::JniWebSocketConnector(string const& JavaClassPath)
{
    NNT_CLASS_CONSTRUCT();
}

JniWebSocketConnector::~JniWebSocketConnector()
{
    NNT_CLASS_DESTORY();
}

void JniWebSocketConnector::close()
{

}

bool JniWebSocketConnector::connect()
{
    return false;
}

bool JniWebSocketConnector::write(memory_type const& mem)
{
    return false;
}

WebSocketConnector::buffer_type JniWebSocketConnector::wait()
{
    return ::NNT_NS::Nil<buffer_type>();
}

CROSS_END