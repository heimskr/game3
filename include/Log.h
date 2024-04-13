#pragma once

#include <format>
#include <iostream>
#include <mutex>
#include <string>

// #define NO_LOGS

namespace Game3::Logger {
	extern std::mutex mutex;
	extern int level;
	std::string getTimestamp();
}

#ifdef NO_LOGS
#define INFOX_(message)    do {} while (false)
#define WARNX_(message)    do {} while (false)
#define ERRORX_(message)   do {} while (false)
#define SPAMX_(message)    do {} while (false)
#define SUCCESSX_(message) do {} while (false)
#else
#define LOG_START "\e[2m[\e[1m"
#define LOG_INFO_MIDDLE "\e[22;2m]\e[22m (\e[22;1;34mi\e[22;39m)\e[2m ::\e[22m "
#define LOG_WARN_MIDDLE "\e[22;2m]\e[22m (\e[22;1;33m!\e[22;39m)\e[2m ::\e[22m "
#define LOG_ERROR_MIDDLE "\e[22;2m]\e[22m (\e[22;1;31m!\e[22;39m)\e[2m ::\e[22m "
#define LOG_SPAM_MIDDLE "\e[22;2m]\e[22m (\e[22;1;35m_\e[22;39m)\e[2m :: "
#define LOG_SUCCESS_MIDDLE "\e[22;2m]\e[22m (\e[22;1;32m🗸\e[22;39m)\e[2m :: \e[22;32m"
#define INFOX_(lvl, message) \
	do { if (Game3::Logger::level < (lvl)) break; \
	     std::unique_lock lock(Game3::Logger::mutex); \
	     std::cerr << LOG_START << ::Game3::Logger::getTimestamp() \
	               << LOG_INFO_MIDDLE << message << '\n'; } while (false)

#define WARNX_(lvl, message) \
	do { if (Game3::Logger::level < (lvl)) break; \
	     std::unique_lock lock(Game3::Logger::mutex); \
	     std::cerr << LOG_START << ::Game3::Logger::getTimestamp() \
	               << LOG_WARN_MIDDLE << message << '\n'; } while (false)

#define ERRORX_(lvl, message) \
	do { if (Game3::Logger::level < (lvl)) break; \
	     std::unique_lock lock(Game3::Logger::mutex); \
	     std::cerr << LOG_START << ::Game3::Logger::getTimestamp() \
	               << LOG_ERROR_MIDDLE << message << '\n'; } while (false)

#define SPAMX_(lvl, message) \
	do { if (Game3::Logger::level < (lvl)) break; \
	     std::unique_lock lock(Game3::Logger::mutex); \
	     std::cerr << LOG_START << ::Game3::Logger::getTimestamp() \
	               << LOG_SPAM_MIDDLE << message << "\e[22m\n"; } while (false)

#define SUCCESSX_(lvl, message) \
	do { if (Game3::Logger::level < (lvl)) break; \
	     std::unique_lock lock(Game3::Logger::mutex); \
	     std::cerr << LOG_START << ::Game3::Logger::getTimestamp() \
	               << LOG_SUCCESS_MIDDLE << message << "\e[39m\n"; } while (false)
#endif

#ifdef NO_LOGS
#define INFOX(fmt, ...)    do {} while (false)
#define WARNX(fmt, ...)    do {} while (false)
#define ERRORX(fmt, ...)   do {} while (false)
#define SPAMX(fmt, ...)    do {} while (false)
#define SUCCESSX(fmt, ...) do {} while (false)
#else
#define INFOX(lvl, fmt, ...) \
	do { if (Game3::Logger::level < (lvl)) break; \
	     std::unique_lock lock(Game3::Logger::mutex); \
	     std::cerr << LOG_START << ::Game3::Logger::getTimestamp() << LOG_INFO_MIDDLE \
	               << std::format(fmt, __VA_ARGS__) << '\n'; } while (false)

#define WARNX(lvl, fmt, ...) \
	do { if (Game3::Logger::level < (lvl)) break; \
	     std::unique_lock lock(Game3::Logger::mutex); \
	     std::cerr << LOG_START << ::Game3::Logger::getTimestamp() << LOG_WARN_MIDDLE \
	               << std::format(fmt, __VA_ARGS__) << '\n'; } while (false)

#define ERRORX(lvl, fmt, ...) \
	do { if (Game3::Logger::level < (lvl)) break; \
	     std::unique_lock lock(Game3::Logger::mutex); \
	     std::cerr << LOG_START << ::Game3::Logger::getTimestamp() << LOG_ERROR_MIDDLE \
	               << std::format(fmt, __VA_ARGS__) << '\n'; } while (false)

#define SPAMX(lvl, fmt, ...) \
	do { if (Game3::Logger::level < (lvl)) break; \
	     std::unique_lock lock(Game3::Logger::mutex); \
	     std::cerr << LOG_START << ::Game3::Logger::getTimestamp() << LOG_SPAM_MIDDLE \
	               << std::format(fmt, __VA_ARGS__) << "\e[22m\n"; } while (false)

#define SUCCESSX(lvl, fmt, ...) \
	do { if (Game3::Logger::level < (lvl)) break; \
	     std::unique_lock lock(Game3::Logger::mutex); \
	     std::cerr << LOG_START << ::Game3::Logger::getTimestamp() << LOG_SUCCESS_MIDDLE \
	               << std::format(fmt, __VA_ARGS__) << "\e[39m\n"; } while (false)
#endif

#define INFO(fmt, ...)    INFOX(1, fmt, __VA_ARGS__)
#define WARN(fmt, ...)    WARNX(1, fmt, __VA_ARGS__)
#define ERROR(fmt, ...)   ERRORX(1, fmt, __VA_ARGS__)
#define SPAM(fmt, ...)    SPAMX(1, fmt, __VA_ARGS__)
#define SUCCESS(fmt, ...) SUCCESSX(1, fmt, __VA_ARGS__)
#define INFO_(message)    INFOX_(1, message)
#define WARN_(message)    WARNX_(1, message)
#define ERROR_(message)   ERRORX_(1, message)
#define SPAM_(message)    SPAMX_(1, message)
#define SUCCESS_(message) SUCCESSX_(1, message)

#undef SPAM
#undef SPAM_
#define SPAM(fmt, ...)
#define SPAM_(message)
