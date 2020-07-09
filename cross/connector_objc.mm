#import "cross.hpp"
#import "connector_objc.hpp"
#import <sstream>
#import "url.hpp"
#import "json.hpp"
#import "xml.hpp"
#import "logger.hpp"
#import "stringbuilder.hpp"
#import "threads.hpp"
#import "code.hpp"
#import "memory.hpp"

CROSS_BEGIN

class ObjcHttpConnectorPrivate
{
public:
    
    explicit ObjcHttpConnectorPrivate(ObjcHttpConnector* owner)
    : d_owner(owner)
    {

    }
    
    void clear()
    {
        errcode = -1;
        errmsg.clear();
        buffer.clear();
        rspheaders.clear();
    }
    
    void close()
    {
        clear();
        
        /*
        if (task) {
            [task cancel];
            task = nil;
        }
         */
    }
    
    int errcode = -1;
    string errmsg;
    unsigned short respcode;
    HttpConnector::stream_type buffer;
    HttpConnector::args_type rspheaders;
    
    ObjcHttpConnector *d_owner;
    semaphore sema_task;
};

ObjcHttpConnector::ObjcHttpConnector()
{
    NNT_CLASS_CONSTRUCT(this);
}

ObjcHttpConnector::~ObjcHttpConnector()
{
    d_ptr->close();
    NNT_CLASS_DESTROY();
}

void ObjcHttpConnector::close()
{
    d_ptr->close();
}

NSString* build_query(HttpConnector::args_type const& args)
{
    NSMutableArray *arr = [NSMutableArray array];
    for (auto& e : args)
    {
        auto k = [NSString stringWithUTF8String:e.first.c_str()];
        ::std::ostringstream oss;
        auto vstr = oss.str();
        auto v = [NSString stringWithUTF8String:vstr.c_str()].escape;
        [arr addObject:[NSString stringWithFormat:@"%@=%@", k, v]];
    }
    return [arr componentsJoinedByString:@"&"];
}

NSDictionary* args2dict(HttpConnector::args_type const& args)
{
    NSMutableDictionary *r = [NSMutableDictionary dictionaryWithCapacity:args.size()];
    for (auto &e:args) {
        auto k = [NSString stringWithUTF8String:e.first.c_str()];
        ::std::ostringstream oss;
        oss << e.second;
        auto vstr = oss.str();
        auto v = [NSString stringWithUTF8String:vstr.c_str()];
        [r setValue:v forKey:k];
    }
    return r;
}

void dict2args(NSDictionary *dict, HttpConnector::args_type& args)
{
    for (NSString *k in dict) {
        NSString *v = [dict valueForKey:k];
        string ck = k.UTF8String;
        string cv = v.UTF8String;
        args[ck] = make_property(cv);
    }
}

bool ObjcHttpConnector::send() const
{
    NSMutableString *url = [NSMutableString stringWithUTF8String:this->url.c_str()];

    if (method == Method::GET)
    {
        if (!_reqargs.empty())
        {
            if ([url rangeOfString:@"?"].location == NSNotFound)
            {
                [url appendString:@"/?"];
            }
            else
            {
                [url appendString:@"&"];
            }
            [url appendString:build_query(_reqargs)];
        }
    }
    
    NSMutableURLRequest *req = nil;
    
    if (Mask::IsSet(method, Method::POST) && 0)
    {
        switch (method)
        {
        default:break;
        case Method::POST:
        {
            _reqheaders[HEADER_CONTENT_TYPE] = make_property("multipart/form-data");
            auto args = args2dict(_reqargs);
            NSError *err;
            if (err) {
                Logger::Warn(stringbuilder().space(" ") << "HttpConnector:" << err.code << err.debugDescription.UTF8String);
                return false;
            }
        }
            break;
        case Method::POST_URLENCODED:
        {
            _reqheaders[HEADER_CONTENT_TYPE] = make_property("application/x-www-form-urlencoded; charset=utf-8;");
            req = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:url]];
            [req setHTTPBody:[build_query(_reqargs) dataUsingEncoding:NSUTF8StringEncoding]];
        }
            break;
        case Method::POST_JSON:
        {
            _reqheaders[HEADER_CONTENT_TYPE] = make_property("application/json; charset=utf-8;");
            auto args = args2dict(_reqargs);
            NSError *err;
            if (err) {
                Logger::Warn(stringbuilder().space(" ") << "HttpConnector:" << err.code << err.debugDescription.UTF8String);
                return false;
            }
        }
            break;
        case Method::POST_XML:
        {
            _reqheaders[HEADER_CONTENT_TYPE] = make_property("application/xml; charset=utf-8;");
            auto p = Combine(_reqargs);
            auto val = property_toxml(*p);
            req = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:url]];
            [req setHTTPBody:[[NSString stringWithUTF8String:val.c_str()] dataUsingEncoding:NSUTF8StringEncoding]];
        }
            break;
        }
    } else {
        req = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:url]];
    }
        
    [req setTimeoutInterval:timeout];
    
    // 设置请求头
    if (!_reqheaders.empty() && 0)
    {
        for (auto& e : _reqheaders)
        {
            ::std::ostringstream oss;
            oss << e.first << ": " << e.second;
            auto val = oss.str();
            [req setValue:[NSString stringWithUTF8String:val.c_str()]
       forHTTPHeaderField:[NSString stringWithUTF8String:e.first.c_str()]];
        }
    }
    
    // 取消证书验证
    
    on_connected();
    
    NSError *err;
    if (err) {
        d_ptr->errcode = (int)err.code;
        string msg = err.description.UTF8String;
        d_ptr->errmsg = msg;
        on_error(error(Code::FAILED, msg));
        
        Logger::Error(stringbuilder().space(" ") << "HttpConnector:" << err.code << err.debugDescription.UTF8String);
        d_ptr->sema_task.notify();
        //return;
    }
    
    //d_ptr->respcode = respn.statusCode;
    //    dict2args(respn.allHeaderFields, d_ptr->rspheaders);
    
    on_completed();
    d_ptr->sema_task.notify();
    
    //[d_ptr->task resume];
    
    d_ptr->sema_task.wait();
    //d_ptr->task = nil;
    
    on_disconnected();

    return RespondCodeIsOk(d_ptr->respcode);
}

int ObjcHttpConnector::errcode() const
{
    return d_ptr->errcode;
}

string const& ObjcHttpConnector::errmsg() const
{
    return d_ptr->errmsg;
}

HttpConnector::body_stream_type ObjcHttpConnector::body() const
{
    return d_ptr->buffer;
}

HttpConnector::args_type const& ObjcHttpConnector::respheaders() const
{
    return d_ptr->rspheaders;
}

unsigned short ObjcHttpConnector::respcode() const
{
    return d_ptr->respcode;
}

// ------------------------------------- Download

class ObjcDownloadConnectorPrivate
{
public:
    
    void clear()
    {
        errcode = -1;
        errmsg.clear();
        buffer.clear();
        rspheaders.clear();
    }
    
    void close()
    {
        clear();
    }
    
    int errcode = -1;
    string errmsg;
    unsigned short respcode;
    HttpConnector::stream_type buffer;
    HttpConnector::args_type rspheaders;
};

ObjcDownloadConnector::ObjcDownloadConnector()
{
    NNT_CLASS_CONSTRUCT();
}

ObjcDownloadConnector::~ObjcDownloadConnector()
{
    NNT_CLASS_DESTROY();
}

void ObjcDownloadConnector::close()
{
    
}

bool ObjcDownloadConnector::send() const
{
    return false;
}

int ObjcDownloadConnector::errcode() const
{
    return d_ptr->errcode;
}

string const& ObjcDownloadConnector::errmsg() const
{
    return d_ptr->errmsg;
}

HttpConnector::args_type const& ObjcDownloadConnector::respheaders() const
{
    return d_ptr->rspheaders;
}

unsigned short ObjcDownloadConnector::respcode() const
{
    return d_ptr->respcode;
}

CROSS_END
