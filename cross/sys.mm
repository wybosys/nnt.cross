#import "cross.hpp"
#import "sys.hpp"
#include "logger.hpp"
#include <thread>

CROSS_BEGIN

static string tls_thread_name;

string get_thread_name()
{
    if (tls_thread_name.empty())
    {
        tls_thread_name = [NSThread currentThread].name.UTF8String;
    }
    return tls_thread_name;
}

void set_thread_name(string const& name)
{
    if (tls_thread_name == name)
        return;
    tls_thread_name = name;
    [[NSThread currentThread] setName:[NSString stringWithUTF8String:tls_thread_name.c_str()]];
}

CROSS_END
