#pragma once

#include <memory>

namespace Game3 {
	template <typename T>
	class HeapObject {
		private:
			std::unique_ptr<T> pointer;

		public:
			HeapObject():
				pointer(std::make_unique<T>()) {}

			HeapObject(T &&item):
				pointer(std::make_unique<T>(std::move(item))) {}

			HeapObject(const T &item):
				pointer(std::make_unique<T>(item)) {}

			HeapObject(std::unique_ptr<T> &&item):
				pointer(std::move(item)) {}

			HeapObject(const HeapObject<T> &other):
				pointer(std::make_unique<T>(*other.pointer)) {}

			HeapObject(HeapObject<T> &&other):
				pointer(std::make_unique<T>(std::move(*other.pointer))) {}

			inline HeapObject & operator=(const HeapObject<T> &other) {
				pointer = std::make_unique<T>(*other.pointer);
				return *this;
			}

			inline HeapObject & operator=(HeapObject<T> &&other) {
				pointer = std::make_unique<T>(std::move(*other.pointer));
				return *this;
			}

			inline operator T &() { return *pointer; }
			inline operator const T &() const { return *pointer; }
			inline T & operator*()  { return *pointer; }
			inline T * operator->() { return pointer.get(); }
			inline const T & operator*()  const { return *pointer; }
			inline const T * operator->() const { return pointer.get(); }

			inline bool operator==(const T &other) const {
				return *pointer == other;
			}

			inline bool operator==(const HeapObject<T> &other) const {
				return *pointer == *other.pointer;
			}

			inline operator bool() const {
				return static_cast<bool>(*pointer);
			}
	};
}
