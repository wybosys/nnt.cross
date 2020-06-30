#include "cross.hpp"
#include "connector_boost.hpp"
#include "threads.hpp"
#include "memory.hpp"
#include "url.hpp"
#include "str.hpp"
#include "logger.hpp"
#include "stringbuilder.hpp"
#include <thread>

#include <boost/array.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>

CROSS_BEGIN

#define WS_RESERVE_RECEIVE_BUFFER_SIZE (4096)

using namespace boost;

#define LOG(exp) (stringbuilder() << "BoostWebSocketConnector:" << __LINE__ << " " << exp).str()

class BoostWebSocketConnectorPrivate
{
public:

    BoostWebSocketConnectorPrivate(BoostWebSocketConnector *owner)
        : d_owner(owner), sslctx(beast::net::ssl::context::tls_client)
    {
        // pass
    }

    BoostWebSocketConnector *d_owner;

    bool connect() 
    {
        // 解析地址
        Url url(d_owner->url);
        asio::ip::tcp::resolver resolver(ioc);
        asio::ip::tcp::resolver::results_type res;
        try {
            res = resolver.resolve(url.host, tostr(url.port));
        }
        catch (system::system_error &err) {
            Logger::Critical(LOG(err.what() << " " << d_owner->url));
            d_owner->on_error(error(Code::FAILED, err.what()));
            return false;
        }

        d_owner->on_connecting();

        // 创建连接对象
        try {
            if (url.protocol == "ws:") {
                ws = make_unique<ws_type>(beast::net::make_strand(ioc));
                asio::connect(ws->next_layer().socket(), res.begin(), res.end());
                ws->handshake(url.url(), url.path());
            }
            else {
                wss = make_unique<wss_type>(beast::net::make_strand(ioc), sslctx);
                asio::connect(wss->next_layer().next_layer().socket(), res.begin(), res.end());
                // ssl额外握手
                wss->next_layer().handshake(beast::net::ssl::stream_base::client);
                wss->handshake(url.url(), url.path());
            }
        }
        catch (system::system_error &err) {
            ws = nullptr;
            wss = nullptr;
            Logger::Critical(LOG(err.what()));
            d_owner->on_error(error(Code::FAILED, err.what()));
            return false;
        }

        d_owner->on_connected();

        // 打开异步读取
        if (!wait_bytes())
            return false;

        // 启动线程
        thd_ioc = make_unique<::std::thread>([&]() {
            Logger::Debug(LOG("启动工作线程"));
            ioc.run();
            Logger::Debug(LOG("退出工作线程"));
        });

        return true;
    }

    bool wait_bytes() 
    {
        try {
            if (ws) {
                ws->async_read_some(asio::buffer(buf_read), ::boost::bind(&BoostWebSocketConnectorPrivate::on_bytes, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
            }
            else {
                wss->async_read_some(asio::buffer(buf_read), ::boost::bind(&BoostWebSocketConnectorPrivate::on_bytes, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
            }
        }
        catch (system::system_error &err) {
            ws = nullptr;
            wss = nullptr;
            Logger::Critical(LOG(err.what()));
            d_owner->on_error(error(Code::FAILED, err.what()));
            return false;
        }

        return true;
    }

    void close()
    {
        if (!thd_ioc)
            return;

        try {
            if (ws) {
                system::error_code ec;
                ws->close(beast::websocket::close_code::normal, ec);
                ws = nullptr;
            }

            if (wss) {
                system::error_code ec;
                wss->close(beast::websocket::close_code::normal, ec);
                wss = nullptr;
            }

            if (thd_ioc->joinable()) {
                if (!ioc.stopped()) {
                    ioc.stop();
                }
                thd_ioc->join();
                thd_ioc = nullptr;
            }
        }
        catch (system::system_error &err) {
            ws = nullptr;
            wss = nullptr;
            Logger::Critical(LOG(err.what()));
        }
    }

    void on_bytes(system::error_code ec, size_t len)
    {
        if (!len)
            return;

        NNT_AUTOGUARD(mtx_read);
        auto d = buf_read.data();

        stm_read.clear();
        stm_read.write(d, len);

        Connector::memory_type mem(d, len);
        d_owner->on_bytes(mem);

        sema_read.notify();

        // 释放

        // 重新启动监听
        wait_bytes();
    }

    bool write(Connector::memory_type const &mem)
    {
        size_t out = 0;
        try {
            if (ws) {
                out = ws->write(asio::buffer(mem.buf(), mem.size()));
            }
            else {
                out = wss->write(asio::buffer(mem.buf(), mem.size()));
            }
        }
        catch (system::system_error &err)
        {
            ws = nullptr;
            wss = nullptr;
            Logger::Critical(LOG(err.what()));
            d_owner->on_error(error(Code::FAILED, err.what()));
            return false;
        }
        return out == mem.size();
    }

    // 写排队
    ::std::mutex mtx_write;

    // 锁住异步读取
    ::std::mutex mtx_read;
    semaphore sema_read;

    // 临时读取对象
    ::boost::array<char, WS_RESERVE_RECEIVE_BUFFER_SIZE> buf_read;
    ByteStream<> stm_read;

    // asio主上下文
    asio::io_context ioc;
    beast::net::ssl::context sslctx;

    // asio监听任务
    ::std::unique_ptr<::std::thread> thd_ioc;

    // 连接对象
    typedef beast::websocket::stream<beast::tcp_stream> ws_type;
    typedef beast::websocket::stream<beast::net::ssl::stream<beast::tcp_stream>> wss_type;
    ::std::unique_ptr<ws_type> ws;
    ::std::unique_ptr<wss_type> wss;
};

BoostWebSocketConnector::BoostWebSocketConnector()
{
    NNT_CLASS_CONSTRUCT(this);
}

BoostWebSocketConnector::~BoostWebSocketConnector()
{
    close();
    NNT_CLASS_DESTORY();
}

bool BoostWebSocketConnector::connect()
{
    close();
    return d_ptr->connect();
}

void BoostWebSocketConnector::close()
{
    d_ptr->close();
}

WebSocketConnector::buffer_typep BoostWebSocketConnector::wait()
{
    d_ptr->sema_read.wait();
    NNT_AUTOGUARD(d_ptr->mtx_read);
    return make_shared<buffer_type>(d_ptr->stm_read.buf(), d_ptr->stm_read.size());
}

bool BoostWebSocketConnector::_write(memory_type const &mem)
{
    NNT_AUTOGUARD(d_ptr->mtx_write);
    return d_ptr->write(mem);
}

CROSS_END