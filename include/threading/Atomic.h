#pragma once

#include "net/Buffer.h"

#include <nlohmann/json.hpp>

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

		Atomic & operator=(const nlohmann::json &json) {
			std::atomic<T>::operator=(json.get<T>());
			return *this;
		}
	};

	template <typename T>
	void from_json(const nlohmann::json &json, Atomic<T> &atomic) {
		atomic = json.get<T>();
	}

	template <typename T>
	void to_json(nlohmann::json &json, const Atomic<T> &atomic) {
		json = atomic.load();
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
