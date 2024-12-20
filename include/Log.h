#pragma once

#include <format>
#include <iostream>
#include <print>
#include <string>
#include <syncstream>

// #define NO_LOGS

namespace Game3::Logger {
	extern int level;
	std::string getTimestamp();
}

namespace Game3 {
#ifdef NO_LOGS
	template <typename... Args>
	void INFO(Args &&...) {}
	template <typename... Args>
	void WARN(Args &&...) {}
	template <typename... Args>
	void ERROR(Args &&...) {}
	template <typename... Args>
	void SPAM(Args &&...) {}
	template <typename... Args>
	void SUCCESS(Args &&...) {}
#else
#define LOG_START "\x1b[2m[\x1b[1m"
#define LOG_INFO_MIDDLE "\x1b[22;2m]\x1b[22m (\x1b[22;1;34mi\x1b[22;39m)\x1b[2m ::\x1b[22m "
#define LOG_WARN_MIDDLE "\x1b[22;2m]\x1b[22m (\x1b[22;1;33m!\x1b[22;39m)\x1b[2m ::\x1b[22m "
#define LOG_ERROR_MIDDLE "\x1b[22;2m]\x1b[22m (\x1b[22;1;31m!\x1b[22;39m)\x1b[2m ::\x1b[22m "
#define LOG_SPAM_MIDDLE "\x1b[22;2m]\x1b[22m (\x1b[22;1;35m_\x1b[22;39m)\x1b[2m :: "
#define LOG_SUCCESS_MIDDLE "\x1b[22;2m]\x1b[22m (\x1b[22;1;32m🗸\x1b[22;39m)\x1b[2m :: \x1b[22;32m"
	template <typename... Args>
	void INFO(std::format_string<Args...> format, Args &&...args) {
		std::osyncstream synced{std::cerr};
		std::print(synced, "{}{}{}", LOG_START, Logger::getTimestamp(), LOG_INFO_MIDDLE);
		std::println(synced, format, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void INFO(int level, std::format_string<Args...> format, Args &&...args) {
		if (level <= Logger::level) {
			INFO(format, std::forward<Args>(args)...);
		}
	}

	template <typename... Args>
	void WARN(std::format_string<Args...> format, Args &&...args) {
		std::osyncstream synced{std::cerr};
		std::print(synced, "{}{}{}", LOG_START, Logger::getTimestamp(), LOG_WARN_MIDDLE);
		std::println(synced, format, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void WARN(int level, std::format_string<Args...> format, Args &&...args) {
		if (level <= Logger::level) {
			WARN(format, std::forward<Args>(args)...);
		}
	}

	template <typename... Args>
	void ERROR(std::format_string<Args...> format, Args &&...args) {
		std::osyncstream synced{std::cerr};
		std::print(synced, "{}{}{}", LOG_START, Logger::getTimestamp(), LOG_ERROR_MIDDLE);
		std::println(synced, format, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void ERROR(int level, std::format_string<Args...> format, Args &&...args) {
		if (level <= Logger::level) {
			ERROR(format, std::forward<Args>(args)...);
		}
	}

	template <typename... Args>
	void SPAM(std::format_string<Args...> format, Args &&...args) {
		std::osyncstream synced{std::cerr};
		std::print(synced, "{}{}{}", LOG_START, Logger::getTimestamp(), LOG_SPAM_MIDDLE);
		std::println(synced, format, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void SPAM(int level, std::format_string<Args...> format, Args &&...args) {
		if (level <= Logger::level) {
			SPAM(format, std::forward<Args>(args)...);
		}
	}

	template <typename... Args>
	void SUCCESS(std::format_string<Args...> format, Args &&...args) {
		std::osyncstream synced{std::cerr};
		std::print(synced, "{}{}{}", LOG_START, Logger::getTimestamp(), LOG_SUCCESS_MIDDLE);
		std::print(synced, format, std::forward<Args>(args)...);
		std::println(synced, "\x1b[39m");
	}

	template <typename... Args>
	void SUCCESS(int level, std::format_string<Args...> format, Args &&...args) {
		if (level <= Logger::level) {
			SUCCESS(format, std::forward<Args>(args)...);
		}
	}
#endif
}
