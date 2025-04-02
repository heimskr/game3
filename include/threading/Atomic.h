#pragma once

#include "net/Buffer.h"

#include <boost/json.hpp>

#include <atomic>
#include <format>

namespace Game3 {
	template <typename T>
	struct Atomic: std::atomic<T> {
		using std::atomic<T>::atomic;
		using std::atomic<T>::operator=;
		using std::atomic<T>::operator T;

		Atomic(const Atomic &other):
			std::atomic<T>(other.load()) {}

		Atomic(Atomic &&other):
			std::atomic<T>(other.load()) {}

		Atomic & operator=(const Atomic &other) {
			std::atomic<T>::operator=(other.load());
			return *this;
		}

		Atomic & operator=(Atomic &&other) {
			std::atomic<T>::operator=(other.load());
			return *this;
		}

		Atomic & operator=(const boost::json::value &json) {
			std::atomic<T>::operator=(boost::json::value_to<T>(json));
			return *this;
		}
	};

	template <typename T>
	Atomic<T> tag_invoke(boost::json::value_to_tag<Atomic<T>>, const boost::json::value &json) {
		return {boost::json::value_to<T>(json)};
	}

	template <typename T>
	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, const Atomic<T> &atomic) {
		json = boost::json::value_from(atomic.load());
	}

	template <typename T>
	Buffer & operator<<(Buffer &buffer, const Atomic<T> &atomic) {
		buffer << atomic.load();
		return buffer;
	}

	template <typename T>
	Buffer & operator>>(Buffer &buffer, Atomic<T> &atomic) {
		atomic = buffer.take<T>();
		return buffer;
	}
}

template <typename T>
struct std::formatter<Game3::Atomic<T>> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const auto &atomic, auto &ctx) const {
		return std::format_to(ctx.out(), "{}", atomic.load());
	}
};
