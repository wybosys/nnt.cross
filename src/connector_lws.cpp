#include "cross.hpp"
#include "connector_lws.hpp"
#include "url.hpp"
#include "str.hpp"
#include <mutex>
#include "logger.hpp"
#include <sstream>
#include <thread>
#include <algorithm>
#include <threads.hpp>

#include <libwebsockets.h>
#include <lws_config.h>

CROSS_BEGIN

#define WS_RX_BUFFER_SIZE (65536)
#define WS_RESERVE_RECEIVE_BUFFER_SIZE (4096)
#define WS_RESERVE_WRITE_BUFFER_SIZE (4096)

class LibWebSocketConnectorPrivate {
public:

    LibWebSocketConnector *owner = nullptr;
    struct lws_context *lws = nullptr;
    struct lws *client = nullptr;
    bool iswritable = false;
    bool waitquit = false;

    // 执行监听循环
    shared_ptr<::std::thread> thd_wait;

    // 锁
    ::std::mutex mtx_write, mtx_read;
    semaphore sema_connect, sema_read;

    // 临时读取对象
    ::std::ostringstream stm_read;

    bool connect() {
        const struct lws_protocols PROTOCOLS[] = {
                {
                        "main", ImpSocketCallback, 0, WS_RX_BUFFER_SIZE, 0, this
                },
                {NULL, NULL,                       0, 0}
        };

        lws_context_creation_info info = {0};
        info.port = CONTEXT_PORT_NO_LISTEN;
        info.protocols = PROTOCOLS;
        info.options = LWS_SERVER_OPTION_DISABLE_IPV6;
#if LWS_LIBRARY_VERSION_MAJOR > 1
        info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
#endif
        info.user = this;

        lws = lws_create_context(&info);

        if (!lws) {
            string msg = "lws创建句柄失败";
            Logger::Error(msg);
            owner->on_error(error(Code::FAILED, msg));
            return false;
        }

        owner->on_connecting();

        Url url(owner->url);
        auto tpath = url.path();

        lws_client_connect_info cinfo = {0};
        cinfo.context = lws;
        cinfo.port = url.port;
        cinfo.address = url.host.c_str();
        cinfo.path = tpath.c_str();
        cinfo.host = url.host.c_str();
        cinfo.origin = url.host.c_str();
        if (url.protocol == "wss:") {
#if LWS_LIBRARY_VERSION_MAJOR > 1
            cinfo.ssl_connection = LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED | LCCSCF_ALLOW_INSECURE;
#else
            cinfo.ssl_connection = 1;
#endif
        } else {
            cinfo.ssl_connection = 0;
        }
        cinfo.userdata = this;

        client = lws_client_connect_via_info(&cinfo);
        if (!client) {
            string msg = "lws连接失败";
            Logger::Error(msg);
            owner->on_error(error(Code::FAILED, msg));
            return false;
        }

        // 创建连接线程，和http不同，ws每个连接均会开一a个新线程，但是对外如果不放到线程池中，则表现出阻塞的特性
        thd_wait = make_shared<::std::thread>(ImpWait, this);
        sema_connect.wait();

        return true;
    }

    void close() {
        if (thd_wait && thd_wait->joinable()) {
            waitquit = true;
            thd_wait->join();
            thd_wait = nullptr;
        }

        if (client) {
            lws_close_reason(client, LWS_CLOSE_STATUS_NORMAL, nullptr, 0);
            client = nullptr;
        }

        if (lws) {
            lws_context_destroy(lws);
            lws = nullptr;
        }
    }

    bool write(Connector::memory_type const &mem) {
        if (!iswritable)
            return false;

        char buf[WS_RESERVE_WRITE_BUFFER_SIZE];
        auto &rd = mem.buffer;

        size_t left = mem.size;
        size_t tgtl = ::std::min<size_t>(WS_RESERVE_WRITE_BUFFER_SIZE, left);

        ::std::streampos partl = rd.sgetn(buf, tgtl);
        left -= (size_t) partl;

        while (partl) {

            int p = LWS_WRITE_BINARY;
            if (left)
                p |= LWS_WRITE_NO_FIN;

            auto writed = lws_write(client, (unsigned char *) buf, (size_t) partl, (lws_write_protocol) p);
            if (!writed && !iswritable) {
                return false;
            }

            auto remains = (int) partl - writed;
            if (remains) {
                left += remains;
                rd.pubseekoff(-remains, ::std::ios::cur, ::std::ios::out);
            }

            if (left) {
                tgtl = ::std::min<size_t>(WS_RESERVE_WRITE_BUFFER_SIZE, left);
                partl = rd.sgetn(buf, tgtl);
                left -= (size_t) partl;
            } else {
                break;
            }
        }

        return true;
    }

    static void ImpWait(LibWebSocketConnectorPrivate *self) {
        while (!self->waitquit) {
            if (self->lws) {
                lws_service(self->lws, 30);
                lws_callback_on_writable(self->client);
            }
        }
    }

    void on_connected() {
        owner->on_connected();
        sema_connect.notify();
    }

    void on_error(string const &msg) {
        iswritable = false;

        Logger::Error(msg);
        owner->on_error(error(Code::FAILED, msg));
    }

    void on_disconnected() {
        iswritable = false;

        Logger::Info("lws断开连接");
        owner->on_disconnected();
    }

    void on_bytes(char const *buf, size_t len) {
        if (!len)
            return;

        NNT_AUTOGUARD(mtx_read);

        stm_read.clear();
        stm_read.write(buf, len);

        Connector::memory_type mem(*stm_read.rdbuf());
        mem.from = 0;
        mem.size = len;

        owner->on_bytes(mem);
        sema_read.notify();
    }

    void on_writeable() {
        iswritable = true;
    }

    static int ImpSocketCallback(struct lws *ws, lws_callback_reasons reason, void *user, void *buf, size_t len) {
        if (user == nullptr)
            return 0; // 连接成功之前的全局回调

        auto self = (LibWebSocketConnectorPrivate *) user;
        switch (reason) {
            case LWS_CALLBACK_CLIENT_ESTABLISHED: {
                lws_callback_on_writable(self->client);
                self->on_connected();
            }
                break;
            case LWS_CALLBACK_CLIENT_CONNECTION_ERROR: {
                self->on_error(string((char const *) buf, len));
            }
                break;
            case LWS_CALLBACK_PROTOCOL_DESTROY:
            case LWS_CALLBACK_CLOSED: {
                self->on_disconnected();
            }
                break;
            case LWS_CALLBACK_CLIENT_RECEIVE: {
                self->on_bytes((char const *) buf, len);
            }
                break;
            case LWS_CALLBACK_CLIENT_WRITEABLE: {
                self->on_writeable();
            }
                break;
        }
        return 0;
    }
};

LibWebSocketConnector::LibWebSocketConnector() {
    NNT_CLASS_CONSTRUCT();
    d_ptr->owner = this;
}

LibWebSocketConnector::~LibWebSocketConnector() {
    d_ptr->close();
    NNT_CLASS_DESTORY();
}

bool LibWebSocketConnector::connect() {
    close();
    return d_ptr->connect();
}

void LibWebSocketConnector::close() {
    d_ptr->close();
}

bool LibWebSocketConnector::write(memory_type const &mem) {
    NNT_AUTOGUARD(d_ptr->mtx_write);
    return d_ptr->write(mem);
}

Connector::stream_type const &LibWebSocketConnector::wait() {
    d_ptr->sema_read.wait();

    NNT_AUTOGUARD(d_ptr->mtx_read);
    return *d_ptr->stm_read.rdbuf();
}

CROSS_END
