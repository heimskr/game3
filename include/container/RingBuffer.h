#pragma once

#include <array>
#include <cstddef>
#include <stdexcept>

namespace Game3 {
	template <typename T, size_t C, bool FIFO = true>
	class RingBuffer {
		static_assert(C > 0, "Ring buffer capacity must be positive");

		private:
			std::array<T, C> array{};
			size_t itemCount = 0;
			size_t readPointer = 0;
			size_t writePointer = 0;

			static size_t decrease(size_t &pointer) {
				pointer = (pointer? pointer : C) - 1;
				return pointer;
			}

			static size_t increase(size_t &pointer) {
				if (++pointer == C) {
					pointer = 0;
				}
				return pointer;
			}

		public:
			RingBuffer() = default;

			T pop() {
				if (itemCount == 0) {
					throw std::out_of_range("Ring buffer is empty");
				}

				T out = std::move(array[readPointer]);
				if constexpr (FIFO) {
					increase(readPointer);
				} else {
					decrease(readPointer);
				}
				--itemCount;
				return out;
			}

			void push(T &&item) {
				array[writePointer] = std::move(item);
				increase(writePointer);
				if (itemCount < C) {
					++itemCount;
				}
			}

			inline size_t size() const {
				return itemCount;
			}

			inline bool empty() const {
				return itemCount == 0;
			}
	};
}
