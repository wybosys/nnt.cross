#include "cross.hpp"
#include "sys.hpp"
#include <thread>
#include <sstream>

CROSS_BEGIN

pid_t get_thread_id()
{
    ::std::ostringstream oss;
    oss << ::std::this_thread::get_id();
    return std::stoull(oss.str());
}

CROSS_END