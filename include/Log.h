#pragma once

#include <iostream>
#include <mutex>
#include <string>

namespace Game3 {
	struct Logger {
		std::mutex mutex;

		template <typename T>
		Logger & operator<<(const T &object) {
			std::cerr << object;
			return *this;
		}

		Logger & operator<<(std::ostream & (*fn)(std::ostream &)) {
			fn(std::cerr);
			return *this;
		}

		Logger & operator<<(std::ostream & (*fn)(std::ios &)) {
			fn(std::cerr);
			return *this;
		}

		Logger & operator<<(std::ostream & (*fn)(std::ios_base &)) {
			fn(std::cerr);
			return *this;
		}

		static std::string getTimestamp();
	};

	extern Logger log;
}

#define INFO(message) \
	do { std::unique_lock lock(Game3::log.mutex); \
	     ::Game3::log << "\e[2m[\e[1m" << ::Game3::Logger::getTimestamp() \
	                  << "\e[22;2m]\e[22m (\e[22;1;34mi\e[22;39m)\e[2m ::\e[22m " \
	                  << message << std::endl; } while (false)

#define WARN(message) \
	do { std::unique_lock lock(Game3::log.mutex); \
	     ::Game3::log << "\e[2m[\e[1m" << ::Game3::Logger::getTimestamp() \
	                  << "\e[22;2m]\e[22m (\e[22;1;33m!\e[22;39m)\e[2m ::\e[22m " \
	                  << message << std::endl; } while (false)

#define ERROR(message) \
	do { std::unique_lock lock(Game3::log.mutex); \
	     ::Game3::log << "\e[2m[\e[1m" << ::Game3::Logger::getTimestamp() \
	                  << "\e[22;2m]\e[22m (\e[22;1;31m!\e[22;39m)\e[2m ::\e[22m " \
	                  << message << std::endl; } while (false)

#define SPAM(message) \
	do { std::unique_lock lock(Game3::log.mutex); \
	     ::Game3::log << "\e[2m[\e[1m" << ::Game3::Logger::getTimestamp() \
	                  << "\e[22;2m]\e[22m (\e[22;1;35m_\e[22;39m)\e[2m :: " \
	                  << message << "\e[22m" << std::endl; } while (false)

#define SUCCESS(message) \
	do { std::unique_lock lock(Game3::log.mutex); \
	     ::Game3::log << "\e[2m[\e[1m" << ::Game3::Logger::getTimestamp() \
	                  << "\e[22;2m]\e[22m (\e[22;1;32m🗸\e[22;39m)\e[2m :: \e[22;32m" \
	                  << message << "\e[39m" << std::endl; } while (false)

#undef SPAM
#define SPAM(message)