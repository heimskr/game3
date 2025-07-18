#pragma once

#include <chrono>

namespace Game3 {
	class Time {
		public:
			Time(std::chrono::system_clock::time_point point = std::chrono::system_clock::now()):
				point(point) {}

			template <typename Scale = std::milli>
			auto age() const {
				return std::chrono::duration_cast<std::chrono::duration<int64_t, Scale>>(std::chrono::system_clock::now() - point);
			}

			template <typename Scale = std::milli>
			auto update() {
				std::chrono::system_clock::time_point new_time = std::chrono::system_clock::now();
				auto out = std::chrono::duration_cast<std::chrono::duration<int64_t, Scale>>(new_time - point);
				point = new_time;
				return out;
			}

			template <typename Scale = std::milli>
			auto operator-(const Time &other) {
				return std::chrono::duration_cast<std::chrono::duration<int64_t, Scale>>(point - other.point);
			}

			static Time zero() {
				return {std::chrono::system_clock::time_point{}};
			}

			static Time now() {
				return {};
			}

		private:
			std::chrono::system_clock::time_point point{};
	};
}
