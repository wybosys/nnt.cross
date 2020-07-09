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

USE_CROSS;

@interface OBJC_ObjcHttpConnectorPrivate : NSObject <NSURLSessionDelegate, NSURLSessionTaskDelegate, NSURLSessionDataDelegate, NSURLSessionDownloadDelegate, NSURLSessionStreamDelegate>
{
    size_t target_length; // 目标的大小
}

@property (assign) ObjcHttpConnectorPrivate *d_ptr;

- (void)clear;

@end

CROSS_BEGIN

class ObjcHttpConnectorPrivate
{
public:
    
    explicit ObjcHttpConnectorPrivate(ObjcHttpConnector* owner)
    : d_owner(owner)
    {
        oc_ptr = [OBJC_ObjcHttpConnectorPrivate new];
        oc_ptr.d_ptr = this;
    }
    
    ~ObjcHttpConnectorPrivate()
    {
        oc_ptr = nil;
    }
    
    void clear()
    {
        errcode = -1;
        errmsg.clear();
        buffer.clear();
        rspheaders.clear();
        [oc_ptr clear];
    }
    
    void close()
    {
        clear();
        
        if (task) {
            [task cancel];
            task = nil;
        }
    }
    
    void on_error(error const& e)
    {
        d_owner->on_error(e);
    }
    
    void on_bytes(HttpConnector::memory_type const& mem)
    {
        d_owner->on_bytes(mem);
    }
    
    void on_completed()
    {
        d_owner->on_completed();
    }
    
    void on_connected()
    {
        d_owner->on_connected();
    }
    
    void on_disconnected()
    {
        d_owner->on_disconnected();
    }
    
    void on_progress_download(HttpConnector::progress_type const& range)
    {
        d_owner->on_progress_download(range);
    }
    
    int errcode = -1;
    string errmsg;
    unsigned short respcode;
    HttpConnector::stream_type buffer;
    ::std::mutex mtx_buffer;
    HttpConnector::args_type rspheaders;
    
    ObjcHttpConnector *d_owner;
    semaphore sema_task;
    NSURLSessionDataTask *task = nil;
    OBJC_ObjcHttpConnectorPrivate *oc_ptr;
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
    
    if (Mask::IsSet(method, Method::POST))
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
    
    auto config = [NSURLSessionConfiguration ephemeralSessionConfiguration];
    config.timeoutIntervalForRequest = ctimeout;
    config.timeoutIntervalForResource = timeout;
    config.requestCachePolicy = NSURLRequestReloadIgnoringLocalCacheData;
    req.timeoutInterval = timeout;
    
    // 设置请求头
    if (!_reqheaders.empty())
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
    
    // 创建连接
    NSURLSession *ses = [NSURLSession sessionWithConfiguration:config
                                                      delegate:d_ptr->oc_ptr
                                                 delegateQueue:nil];
    d_ptr->task = [ses dataTaskWithRequest:req];
            
    [d_ptr->task resume];
    d_ptr->sema_task.wait();
    d_ptr->task = nil;

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

// ------------------------------------ OC

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

@implementation OBJC_ObjcHttpConnectorPrivate

- (void)clear
{
    target_length = 0;
}

- (void)on_error:(NSError*)err
{
    _d_ptr->errcode = (int)err.code;
    string msg = err.description.UTF8String;
    _d_ptr->errmsg = msg;
    _d_ptr->on_error(error(Code::FAILED, msg));
    
    Logger::Error(stringbuilder().space(" ") << "HttpConnector:" << err.code << err.debugDescription.UTF8String);
    _d_ptr->sema_task.notify();
}

- (void)URLSession:(NSURLSession *)session didBecomeInvalidWithError:(nullable NSError *)err
{
    [self on_error:err];
}

- (void)URLSession:(NSURLSession *)session didReceiveChallenge:(NSURLAuthenticationChallenge *)challenge
                                             completionHandler:(void (^)(NSURLSessionAuthChallengeDisposition disposition, NSURLCredential * _Nullable credential))completionHandler
{
    if ([challenge.protectionSpace.authenticationMethod isEqualToString:NSURLAuthenticationMethodServerTrust]) {
        // 忽略证书检查
        auto cred = [NSURLCredential credentialForTrust:challenge.protectionSpace.serverTrust];
        completionHandler(NSURLSessionAuthChallengeUseCredential, cred);
        _d_ptr->on_connected();
        return;
    }
    completionHandler(NSURLSessionAuthChallengeCancelAuthenticationChallenge, nil);
    _d_ptr->on_disconnected();
}

- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task
                     willPerformHTTPRedirection:(NSHTTPURLResponse *)response
                                     newRequest:(NSURLRequest *)request
                              completionHandler:(void (^)(NSURLRequest * _Nullable))completionHandler
{
#ifdef NNT_DEBUG
    NSString *msg = [NSString stringWithFormat:@"重定向至 %@", request.URL];
    Logger::Debug(msg.UTF8String);
#endif
    completionHandler(request);
}

- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task
                            didReceiveChallenge:(NSURLAuthenticationChallenge *)challenge
                              completionHandler:(void (^)(NSURLSessionAuthChallengeDisposition disposition, NSURLCredential * _Nullable credential))completionHandler
{
    if ([challenge.protectionSpace.authenticationMethod isEqualToString:NSURLAuthenticationMethodServerTrust]) {
        // 忽略证书检查
        auto cred = [NSURLCredential credentialForTrust:challenge.protectionSpace.serverTrust];
        completionHandler(NSURLSessionAuthChallengeUseCredential, cred);
        return;
    }
    completionHandler(NSURLSessionAuthChallengeCancelAuthenticationChallenge, nil);
}

- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task
                              needNewBodyStream:(void (^)(NSInputStream * _Nullable bodyStream))completionHandler
{
    
}

- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task
                                didSendBodyData:(int64_t)bytesSent
                                 totalBytesSent:(int64_t)totalBytesSent
                       totalBytesExpectedToSend:(int64_t)totalBytesExpectedToSend
{
    
}

- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task
                           didCompleteWithError:(nullable NSError *)err
{
    if (err) {
        [self on_error:err];
    } else {
        _d_ptr->on_completed();
    }
    _d_ptr->on_disconnected();
    _d_ptr->sema_task.notify();
}

- (void)URLSession:(NSURLSession *)session downloadTask:(NSURLSessionDownloadTask *)downloadTask
                              didFinishDownloadingToURL:(NSURL *)location
{
    
}

- (void)URLSession:(NSURLSession *)session downloadTask:(NSURLSessionDownloadTask *)downloadTask
                                           didWriteData:(int64_t)bytesWritten
                                      totalBytesWritten:(int64_t)totalBytesWritten
                              totalBytesExpectedToWrite:(int64_t)totalBytesExpectedToWrite
{
    
}

- (void)URLSession:(NSURLSession *)session downloadTask:(NSURLSessionDownloadTask *)downloadTask
                                      didResumeAtOffset:(int64_t)fileOffset
                                     expectedTotalBytes:(int64_t)expectedTotalBytes
{
    
}

- (void)URLSession:(NSURLSession *)session dataTask:(NSURLSessionDataTask *)dataTask
                                 didReceiveResponse:(NSURLResponse *)response
                                  completionHandler:(void (^)(NSURLSessionResponseDisposition disposition))completionHandler
{
    auto respn = (NSHTTPURLResponse*)response;
    _d_ptr->respcode = respn.statusCode;
    dict2args(respn.allHeaderFields, _d_ptr->rspheaders);
    
    // 获取目标大小
    auto fnd = _d_ptr->rspheaders.find("Content-Length");
    if (fnd != _d_ptr->rspheaders.end()) {
        target_length = fnd->second->toInteger();
    }

    completionHandler(NSURLSessionResponseAllow);
}

- (void)URLSession:(NSURLSession *)session dataTask:(NSURLSessionDataTask *)dataTask
                              didBecomeDownloadTask:(NSURLSessionDownloadTask *)downloadTask
{
    
}

- (void)URLSession:(NSURLSession *)session dataTask:(NSURLSessionDataTask *)dataTask
                                didBecomeStreamTask:(NSURLSessionStreamTask *)streamTask
{
    
}

- (void)URLSession:(NSURLSession *)session dataTask:(NSURLSessionDataTask *)dataTask
                                     didReceiveData:(NSData *)data
{
    size_t lbuf = data.length;
    if (!lbuf)
        return;
    auto buf = (char const*)data.bytes;
    
    NNT_AUTOGUARD(_d_ptr->mtx_buffer);
    _d_ptr->buffer.write(buf, lbuf);

    HttpConnector::memory_type mem(buf, lbuf);
    _d_ptr->on_bytes(mem);
    
    HttpConnector::progress_type down;
    down.from = 0;
    down.size = target_length;
    down.value = _d_ptr->buffer.size();
    _d_ptr->on_progress_download(down);
}

- (void)URLSession:(NSURLSession *)session dataTask:(NSURLSessionDataTask *)dataTask
                                  willCacheResponse:(NSCachedURLResponse *)proposedResponse
                                  completionHandler:(void (^)(NSCachedURLResponse * _Nullable cachedResponse))completionHandler
{
    
    completionHandler(nil);
}

- (void)URLSession:(NSURLSession *)session readClosedForStreamTask:(NSURLSessionStreamTask *)streamTask
{
    
}

- (void)URLSession:(NSURLSession *)session writeClosedForStreamTask:(NSURLSessionStreamTask *)streamTask
{
    
}

- (void)URLSession:(NSURLSession *)session betterRouteDiscoveredForStreamTask:(NSURLSessionStreamTask *)streamTask
{
    
}

- (void)URLSession:(NSURLSession *)session streamTask:(NSURLSessionStreamTask *)streamTask
                                 didBecomeInputStream:(NSInputStream *)inputStream
                                         outputStream:(NSOutputStream *)outputStream
{
    
}

@end

#pragma clang diagnostic pop
