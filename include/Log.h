#pragma once

#include <iostream>
#include <mutex>
#include <string>

// #define NO_LOGS

namespace Game3::Logger {
	extern std::mutex mutex;
	std::string getTimestamp();
}

#ifdef NO_LOGS
#define INFO_(message)    do {} while (false)
#define WARN_(message)    do {} while (false)
#define ERROR_(message)   do {} while (false)
#define SPAM_(message)    do {} while (false)
#define SUCCESS_(message) do {} while (false)
#else
#define INFO_(message) \
	do { std::unique_lock lock(Game3::Logger::mutex); \
	     std::cerr << "\e[2m[\e[1m" << ::Game3::Logger::getTimestamp() \
	               << "\e[22;2m]\e[22m (\e[22;1;34mi\e[22;39m)\e[2m ::\e[22m " \
	               << message << std::endl; } while (false)

#define WARN_(message) \
	do { std::unique_lock lock(Game3::Logger::mutex); \
	     std::cerr << "\e[2m[\e[1m" << ::Game3::Logger::getTimestamp() \
	               << "\e[22;2m]\e[22m (\e[22;1;33m!\e[22;39m)\e[2m ::\e[22m " \
	               << message << std::endl; } while (false)

#define ERROR_(message) \
	do { std::unique_lock lock(Game3::Logger::mutex); \
	     std::cerr << "\e[2m[\e[1m" << ::Game3::Logger::getTimestamp() \
	               << "\e[22;2m]\e[22m (\e[22;1;31m!\e[22;39m)\e[2m ::\e[22m " \
	               << message << std::endl; } while (false)

#define SPAM_(message) \
	do { std::unique_lock lock(Game3::Logger::mutex); \
	     std::cerr << "\e[2m[\e[1m" << ::Game3::Logger::getTimestamp() \
	               << "\e[22;2m]\e[22m (\e[22;1;35m_\e[22;39m)\e[2m :: " \
	               << message << "\e[22m" << std::endl; } while (false)

#define SUCCESS_(message) \
	do { std::unique_lock lock(Game3::Logger::mutex); \
	     std::cerr << "\e[2m[\e[1m" << ::Game3::Logger::getTimestamp() \
	               << "\e[22;2m]\e[22m (\e[22;1;32m🗸\e[22;39m)\e[2m :: \e[22;32m" \
	               << message << "\e[39m" << std::endl; } while (false)
#endif

#ifdef NO_LOGS
#define INFO(fmt, ...)    do {} while (false)
#define WARN(fmt, ...)    do {} while (false)
#define ERROR(fmt, ...)   do {} while (false)
#define SPAM(fmt, ...)    do {} while (false)
#define SUCCESS(fmt, ...) do {} while (false)
#else
#define INFO(fmt, ...) \
	do { std::unique_lock lock(Game3::Logger::mutex); \
	     std::cerr << "\e[2m[\e[1m" << ::Game3::Logger::getTimestamp() \
	               << "\e[22;2m]\e[22m (\e[22;1;34mi\e[22;39m)\e[2m ::\e[22m " \
	               << std::format(fmt, __VA_ARGS__) << std::endl; } while (false)

#define WARN(fmt, ...) \
	do { std::unique_lock lock(Game3::Logger::mutex); \
	     std::cerr << "\e[2m[\e[1m" << ::Game3::Logger::getTimestamp() \
	               << "\e[22;2m]\e[22m (\e[22;1;33m!\e[22;39m)\e[2m ::\e[22m " \
	               << std::format(fmt, __VA_ARGS__) << std::endl; } while (false)

#define ERROR(fmt, ...) \
	do { std::unique_lock lock(Game3::Logger::mutex); \
	     std::cerr << "\e[2m[\e[1m" << ::Game3::Logger::getTimestamp() \
	               << "\e[22;2m]\e[22m (\e[22;1;31m!\e[22;39m)\e[2m ::\e[22m " \
	               << std::format(fmt, __VA_ARGS__) << std::endl; } while (false)

#define SPAM(fmt, ...) \
	do { std::unique_lock lock(Game3::Logger::mutex); \
	     std::cerr << "\e[2m[\e[1m" << ::Game3::Logger::getTimestamp() \
	               << "\e[22;2m]\e[22m (\e[22;1;35m_\e[22;39m)\e[2m :: " \
	               << std::format(fmt, __VA_ARGS__) << "\e[22m" << std::endl; } while (false)

#define SUCCESS(fmt, ...) \
	do { std::unique_lock lock(Game3::Logger::mutex); \
	     std::cerr << "\e[2m[\e[1m" << ::Game3::Logger::getTimestamp() \
	               << "\e[22;2m]\e[22m (\e[22;1;32m🗸\e[22;39m)\e[2m :: \e[22;32m" \
	               << std::format(fmt, __VA_ARGS__) << "\e[39m" << std::endl; } while (false)
#endif

#undef SPAM
#undef SPAM_
#define SPAM(fmt, ...)
#define SPAM_(message)
