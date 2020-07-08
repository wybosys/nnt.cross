#include "cross.hpp"
#include "logger.hpp"

CROSS_BEGIN

NNT_SINGLETON_IMPL(Logger);

void Logger::_shared_init()
{
// pass
}

Logger::Logger()
	: prefix("cross")
{
	// pass
}

void Logger::debug(string const& msg)
{
	log(LogLevel::DEVELOP, msg);
}

void Logger::info(string const& msg)
{
	log(LogLevel::INFO, msg);
}

void Logger::notice(string const& msg)
{
	log(LogLevel::NOTICE, msg);
}

void Logger::warn(string const& msg)
{
	log(LogLevel::WARNING, msg);
}

void Logger::error(string const& msg)
{
	log(LogLevel::FATAL, msg);
}

void Logger::alert(string const& msg)
{
	log(LogLevel::ALERT, msg);
}

void Logger::critical(string const& msg)
{
	log(LogLevel::CRITICAL, msg);
}

void Logger::emergency(string const& msg)
{
	log(LogLevel::EMERGENCY, msg);
}

string Logger::format(LogLevel lv, string const& msg)
{
	return (prefix.empty() ? "" : prefix + ": ") + msg;
}

#if !defined(NNT_DARWIN)

void Logger::log(LogLevel lv, string const& msg)
{
	auto str = format(lv, msg);
	switch (lv)
	{
	case LogLevel::DEVELOP:
	{
		NNT_DEBUG_EXPRESS(cout << str << endl;);
	}
		break;
	case LogLevel::SPECIAL:
	case LogLevel::CUSTOM:
	case LogLevel::INFO:
	case LogLevel::NOTICE:
	{
		cout << str << endl;
	}
		break;
	case LogLevel::WARNING:
	case LogLevel::FATAL:
	case LogLevel::ALERT:
	case LogLevel::CRITICAL:
	case LogLevel::EMERGENCY:
	{
		cerr << str << endl;
	}
		break;
	}
}

#endif

CROSS_END
