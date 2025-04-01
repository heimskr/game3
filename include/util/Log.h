#pragma once

#include <format>
#include <fstream>
#include <iostream>
#include <print>
#include <string>
#include <syncstream>

// #define NO_LOGS
#ifdef __MINGW32__
#define LOG_TO_FILE
#endif

namespace Game3::Logger {
	extern int level;
	std::string getTimestamp();
#ifdef LOG_TO_FILE
	std::ofstream & fileStream();
	std::string stripANSI(std::string_view);
#endif
}

namespace Game3 {
#ifdef NO_LOGS
	template <typename... Args>
	void INFO(Args &&...) {}
	template <typename... Args>
	void WARN(Args &&...) {}
	template <typename... Args>
	void ERR(Args &&...) {}
	template <typename... Args>
	void SPAM(Args &&...) {}
	template <typename... Args>
	void SUCCESS(Args &&...) {}
#else

#if defined(__APPLE__) || defined(__MINGW32__)
#ifdef LOG_TO_FILE
#define DECLARE_STREAMS auto &stream = std::cerr; [[maybe_unused]] auto &logfile = Logger::fileStream();
#else
#define DECLARE_STREAMS auto &stream = std::cerr
#endif
#else
#ifdef LOG_TO_FILE
#define DECLARE_STREAMS std::osyncstream stream{std::cerr}; [[maybe_unused]] auto &logfile = Logger::fileStream();
#else
#define DECLARE_STREAMS std::osyncstream stream{std::cerr}
#endif
#endif

#define LOG_START "\x1b[2m[\x1b[1m"
#define LOG_INFO_MIDDLE "\x1b[22;2m]\x1b[22m (\x1b[22;1;34mi\x1b[22;39m)\x1b[2m ::\x1b[22m "
#define LOG_WARN_MIDDLE "\x1b[22;2m]\x1b[22m (\x1b[22;1;33m!\x1b[22;39m)\x1b[2m ::\x1b[22m "
#define LOG_ERROR_MIDDLE "\x1b[22;2m]\x1b[22m (\x1b[22;1;31m!\x1b[22;39m)\x1b[2m ::\x1b[22m "
#define LOG_SPAM_MIDDLE "\x1b[22;2m]\x1b[22m (\x1b[22;1;35m_\x1b[22;39m)\x1b[2m :: "
#define LOG_SUCCESS_MIDDLE "\x1b[22;2m]\x1b[22m (\x1b[22;1;32m🗸\x1b[22;39m)\x1b[2m :: \x1b[22;32m"

	template <typename... Args>
	void INFO(std::format_string<Args...> format, Args &&...args) {
		DECLARE_STREAMS
#ifdef LOG_TO_FILE
		std::string message = std::format(format, std::forward<Args>(args)...);
		std::string stamp = Logger::getTimestamp();
		std::println(stream, "{}{}{}{}", LOG_START, stamp, LOG_INFO_MIDDLE, message);
		std::println(logfile, "[{}] (i) :: {}", stamp, Logger::stripANSI(message));
#else
		std::print(stream, "{}{}{}", LOG_START, Logger::getTimestamp(), LOG_INFO_MIDDLE);
		std::println(stream, format, std::forward<Args>(args)...);
#endif
	}

	template <typename... Args>
	void INFO(int level, std::format_string<Args...> format, Args &&...args) {
		if (level <= Logger::level) {
			INFO(format, std::forward<Args>(args)...);
		}
	}

	template <typename... Args>
	void WARN(std::format_string<Args...> format, Args &&...args) {
		DECLARE_STREAMS
#ifdef LOG_TO_FILE
		std::string message = std::format(format, std::forward<Args>(args)...);
		std::string stamp = Logger::getTimestamp();
		std::println(stream, "{}{}{}{}", LOG_START, stamp, LOG_WARN_MIDDLE, message);
		std::println(logfile, "[{}] (w) :: {}", stamp, Logger::stripANSI(message));
#else
		std::print(stream, "{}{}{}", LOG_START, Logger::getTimestamp(), LOG_WARN_MIDDLE);
		std::println(stream, format, std::forward<Args>(args)...);
#endif
	}

	template <typename... Args>
	void WARN(int level, std::format_string<Args...> format, Args &&...args) {
		if (level <= Logger::level) {
			WARN(format, std::forward<Args>(args)...);
		}
	}

	template <typename... Args>
	void ERR(std::format_string<Args...> format, Args &&...args) {
		DECLARE_STREAMS
#ifdef LOG_TO_FILE
		std::string message = std::format(format, std::forward<Args>(args)...);
		std::string stamp = Logger::getTimestamp();
		std::println(stream, "{}{}{}{}", LOG_START, stamp, LOG_ERROR_MIDDLE, message);
		std::println(logfile, "[{}] (e) :: {}", stamp, Logger::stripANSI(message));
#else
		std::print(stream, "{}{}{}", LOG_START, Logger::getTimestamp(), LOG_ERROR_MIDDLE);
		std::println(stream, format, std::forward<Args>(args)...);
#endif
	}

	template <typename... Args>
	void ERR(int level, std::format_string<Args...> format, Args &&...args) {
		if (level <= Logger::level) {
			ERR(format, std::forward<Args>(args)...);
		}
	}

	template <typename... Args>
	void SPAM(std::format_string<Args...> format, Args &&...args) {
		DECLARE_STREAMS
#ifdef LOG_TO_FILE
		std::string message = std::format(format, std::forward<Args>(args)...);
		std::string stamp = Logger::getTimestamp();
		std::println(stream, "{}{}{}{}", LOG_START, stamp, LOG_SPAM_MIDDLE, message);
		std::println(logfile, "[{}] (s) :: {}", stamp, Logger::stripANSI(message));
#else
		std::print(stream, "{}{}{}", LOG_START, Logger::getTimestamp(), LOG_SPAM_MIDDLE);
		std::println(stream, format, std::forward<Args>(args)...);
#endif
	}

	template <typename... Args>
	void SPAM(int level, std::format_string<Args...> format, Args &&...args) {
		if (level <= Logger::level) {
			SPAM(format, std::forward<Args>(args)...);
		}
	}

	template <typename... Args>
	void SUCCESS(std::format_string<Args...> format, Args &&...args) {
		DECLARE_STREAMS
#ifdef LOG_TO_FILE
		std::string message = std::format(format, std::forward<Args>(args)...);
		std::string stamp = Logger::getTimestamp();
		std::println(stream, "{}{}{}{}\x1b[39m", LOG_START, stamp, LOG_SUCCESS_MIDDLE, message);
		std::println(logfile, "[{}] (i) :: {}", stamp, Logger::stripANSI(message));
#else
		std::print(stream, "{}{}{}", LOG_START, Logger::getTimestamp(), LOG_SUCCESS_MIDDLE);
		std::print(stream, format, std::forward<Args>(args)...);
		std::println(stream, "\x1b[39m");
#endif
	}

	template <typename... Args>
	void SUCCESS(int level, std::format_string<Args...> format, Args &&...args) {
		if (level <= Logger::level) {
			SUCCESS(format, std::forward<Args>(args)...);
		}
	}
#endif
#undef DECLARE_STREAMS
}
