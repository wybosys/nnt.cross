#include "cross.hpp"
#include "sys.hpp"
#include <thread>
#include <sstream>

CROSS_BEGIN

pid_t get_thread_id()
{
    ::std::stringstream oss;
    oss << ::std::this_thread::get_id();
    pid_t r;
    oss >> r;
    return r;
}

CROSS_END