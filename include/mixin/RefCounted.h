#pragma once

#include <cassert>
#include <utility>

namespace Game3 {
	template <typename Sub>
	class Ref {
		public:
			Ref(Sub *pointer):
				pointer(pointer) {
					pointer->ref();
				}

			Ref(const Ref &other):
				pointer(other.pointer) {
					if (pointer) {
						pointer->ref();
					}
				}

			Ref(Ref &&other):
				pointer(std::exchange(other.pointer, nullptr)) {}

			~Ref() {
				if (pointer) {
					pointer->deref();
				}
			}

			Ref & operator=(const Ref &other) {
				if (this == &other || pointer == other.pointer){
					return *this;
				}

				if (pointer) {
					pointer->deref();
				}

				pointer = other.pointer;

				if (pointer) {
					pointer->ref();
				}
			}

			Ref & operator=(Ref &&other) {
				if (pointer) {
					pointer->deref();
				}

				pointer = std::exchange(other.pointer, nullptr);
				return *this;
			}

			bool operator==(const Ref &other) const {
				return pointer == other.pointer;
			}

			Sub & operator*() const {
				return *pointer;
			}

			Sub * operator->() const {
				return pointer;
			}

		private:
			Sub *pointer = nullptr;
	};

	template <typename Sub>
	class RefCounted {
		public:
			auto getRefCount() const {
				return refCount;
			}

			void ref() {
				++refCount;
			}

			bool deref() {
				if (--refCount == 0) {
					delete static_cast<Sub *>(this);
					return true;
				}
				return false;
			}

			Ref<Sub> getRef() {
				return {static_cast<Sub *>(this)};
			}

			~RefCounted() {
				assert(refCount == 0);
			}

		protected:
			RefCounted() = default;

		private:
			int refCount = 1;
	};
}
