#include "cross.h"
#include "connector_lws.h"
#include "url.h"
#include "str.h"
#include <mutex>
#include "logger.h"
#include <sstream>

#include <lws/libwebsockets.h>

#ifdef NNT_WINDOWS
#pragma comment(lib,"ws2_32.lib")
#endif

CROSS_BEGIN

#define WS_RX_BUFFER_SIZE (65536)
#define WS_RESERVE_RECEIVE_BUFFER_SIZE (4096)
#define WS_RESERVE_WRITE_BUFFER_SIZE (4096)

class LibWebSocketConnectorPrivate
{
public:

    bool connect();
    void close();
    bool write(Connector::memory_type const& mem);

    static int ImpSocketCallback(struct lws *ws, lws_callback_reasons reason, void *user, void *buf, size_t len);

    LibWebSocketConnector *owner = nullptr;
    lws_context *lws = nullptr;
    struct lws *client = nullptr;
    mutex mtx_write, mtx_read;
    bool iswritable = false;

    void on_connected();
    void on_error();
    void on_disconnected();
    void on_bytes(char const* buf, size_t len);
    void on_writeable();
};

static const struct lws_protocols PROTOCOLS[] = {
{
    "ws",
    LibWebSocketConnectorPrivate::ImpSocketCallback,
    0,
    WS_RX_BUFFER_SIZE,
},
{ NULL, NULL, 0, 0 }
};

bool LibWebSocketConnectorPrivate::connect() {
    if (lws) {
        Logger::Error("��ǰlws�Ѿ���������");
        return false;
    }

    lws_context_creation_info info = { 0 };
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = PROTOCOLS;
    info.gid = -1;
    info.uid = -1;
    info.options = 0;
    info.user = this;
        
    lws = lws_create_context(&info);

    if (!lws) {
        string msg = "����lws����";
        Logger::Error(msg);
        owner->on_error(error(Code::FAILED, msg));
        return false;
    }

    owner->on_connecting();

    Url url(owner->url);
    string ads_port = url.host + ":" + tostr(url.port);
    int usessl = url.protocol == "ws:" ? 0 : 1;

    vector<string> names;
    for (auto &e:PROTOCOLS) {
        if (e.callback)
            names.emplace_back(e.name);
    }
    auto name = implode(names, ", ");

    client = lws_client_connect(lws,
        url.host.c_str(), url.port, usessl,
        url.path().c_str(), ads_port.c_str(), ads_port.c_str(),
        name.c_str(), -1);

    if (!client) {
        string msg = "lws����ʧ��";
        Logger::Error(msg);
        owner->on_error(error(Code::FAILED, msg));
        return false;
    }

    return true;
}

void LibWebSocketConnectorPrivate::close()
{
    if (client) {
        lws_close_reason(client, LWS_CLOSE_STATUS_NORMAL, nullptr, 0);
        client = nullptr;
    }

    if (lws) {
        lws_context_destroy(lws);
        lws = nullptr;
    }
}

bool LibWebSocketConnectorPrivate::write(Connector::memory_type const& mem)
{
    // NNT_AUTOGUARD(mtx_write); �����е����Ѿ��ӹ�����
    if (!iswritable)
        return false;

    char buf[WS_RESERVE_WRITE_BUFFER_SIZE];
    auto& rd = mem.buffer;

    size_t left = mem.size;
    size_t tgtl = min(WS_RESERVE_WRITE_BUFFER_SIZE, left);

    streampos partl = rd.sgetn(buf, tgtl);
    left -= (size_t)partl;

    while (partl) {

        int p = LWS_WRITE_BINARY;
        if (left)
            p |= LWS_WRITE_NO_FIN;

        auto writed = lws_write(client, (unsigned char*)buf, (size_t)partl, (lws_write_protocol)p);
        if (!writed && !iswritable) {
            // ����
            return false;
        }

        auto remains = (int)partl - writed;
        if (remains) {
            left += remains;
            rd.pubseekoff(-remains, ios_base::cur, ios_base::out);
        }

        if (left) {
            tgtl = min(WS_RESERVE_WRITE_BUFFER_SIZE, left);
            partl = rd.sgetn(buf, tgtl);
            left -= (size_t)partl;
        }
        else {
            // �������
            break;
        }
    }

    return true;
}


int LibWebSocketConnectorPrivate::ImpSocketCallback(struct lws *ws, lws_callback_reasons reason, void *user, void *buf, size_t len)
{
    auto self = (LibWebSocketConnectorPrivate*)user;
    switch (reason) {
    case LWS_CALLBACK_CLIENT_ESTABLISHED: {
        lws_callback_on_writable(self->client);
        self->on_connected();
    } break;
    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        self->on_error();
        break;
    case LWS_CALLBACK_PROTOCOL_DESTROY:
    case LWS_CALLBACK_CLOSED:
        self->on_disconnected();
        break;
    case LWS_CALLBACK_CLIENT_RECEIVE:
        self->on_bytes((char const*)buf, len);
        break;
    case LWS_CALLBACK_CLIENT_WRITEABLE:
        self->on_writeable();
        break;
    default:
        break;
    }
    return 0;
}

void LibWebSocketConnectorPrivate::on_connected()
{
    owner->on_connected();
}

void LibWebSocketConnectorPrivate::on_error()
{
    NNT_AUTOGUARD(mtx_write);
    iswritable = false;

    string msg = "lws������������";
    Logger::Error(msg);
    owner->on_error(error(Code::FAILED, msg));
}

void LibWebSocketConnectorPrivate::on_disconnected()
{
    NNT_AUTOGUARD(mtx_write);
    iswritable = false;        

    Logger::Info("lws�Ͽ�����");
    owner->on_disconnected();
}

void LibWebSocketConnectorPrivate::on_bytes(char const* buf, size_t len)
{
    if (!len)
        return;

    NNT_AUTOGUARD(mtx_read);
    stringstream stm;
    stm.write(buf, len);

    Connector::memory_type mem(*stm.rdbuf());
    mem.from = 0;
    mem.size = len;

    owner->on_bytes(mem);
}

void LibWebSocketConnectorPrivate::on_writeable()
{
    NNT_AUTOGUARD(mtx_write);
    iswritable = true;
}

LibWebSocketConnector::LibWebSocketConnector()
{
    NNT_CLASS_CONSTRUCT();
    d_ptr->owner = this;
}

LibWebSocketConnector::~LibWebSocketConnector()
{
    d_ptr->close();
    NNT_CLASS_DESTORY();
}

bool LibWebSocketConnector::connect()
{
    return d_ptr->connect();
}

void LibWebSocketConnector::close()
{
    d_ptr->close();
}

bool LibWebSocketConnector::write(memory_type const& mem)
{
    NNT_AUTOGUARD(d_ptr->mtx_write);
    return d_ptr->write(mem);
}

CROSS_END