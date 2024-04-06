#pragma once

// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <v8.h>

#include <cassert>
#include <memory>

namespace Game3 {
	template <typename T>
	class ObjectWrap {
		public:
			std::shared_ptr<T> object;

			ObjectWrap(std::shared_ptr<T> object_ = {}):
				object(std::move(object_)) {}

			virtual ~ObjectWrap() {
				if (persistent.IsEmpty())
					return;
				persistent.ClearWeak();
				persistent.Reset();
			}

			template <typename... Args>
			static ObjectWrap<T> * make(Args &&...args) {
				return new ObjectWrap<T>(std::make_shared<T>(std::forward<Args>(args)...));
			}

			static inline ObjectWrap<T> & unwrap(v8::Handle<v8::Object> handle) {
				assert(!handle.IsEmpty());
				assert(handle->InternalFieldCount() > 1);
				void *ptr = handle->GetAlignedPointerFromInternalField(1);
				return *static_cast<ObjectWrap<T> *>(ptr);
			}

			static inline std::shared_ptr<T> get(v8::Handle<v8::Object> handle) {
				return unwrap(handle).object;
			}

			inline T & operator*() const {
				assert(object);
				return *object;
			}

			inline T * operator->() const {
				assert(object);
				return object.get();
			}

			inline v8::Local<v8::Object> handle(v8::Isolate *isolate) {
				return v8::Local<v8::Object>::New(isolate, persistent);
			}

			inline v8::Persistent<v8::Object> & getPersistent() {
				return persistent;
			}

			inline void wrap(v8::Isolate *isolate, const char *internal_name, v8::Handle<v8::Object> handle) {
				assert(persistent.IsEmpty());
				assert(handle->InternalFieldCount() > 1);
				handle->SetAlignedPointerInInternalField(0, const_cast<void *>(reinterpret_cast<const void *>(internal_name)));
				handle->SetAlignedPointerInInternalField(1, this);
				persistent.Reset(isolate, handle);
				makeWeak();
			}

		protected:
			int refs = 0;

			inline void makeWeak(void) {
				persistent.SetWeak(this, weakCallback, v8::WeakCallbackType::kParameter);
			}

			/* Ref() marks the object as being attached to an event loop.
			 * Refed objects will not be garbage collected, even if
			 * all references are lost.
			 */
			void ref() {
				assert(!persistent.IsEmpty());
				persistent.ClearWeak();
				++refs;
			}

			/* Unref() marks an object as detached from the event loop.  This is its
			 * default state.  When an object with a "weak" reference changes from
			 * attached to detached state it will be freed. Be careful not to access
			 * the object after making this call as it might be gone!
			 * (A "weak reference" means an object that only has a
			 * persistent handle.)
			 *
			 * DO NOT CALL THIS FROM DESTRUCTOR
			 */
			void unref() {
				assert(!persistent.IsEmpty());
				assert(!persistent.IsWeak());
				assert(refs > 0);

				if (--refs == 0)
					makeWeak();
			}

		private:
			v8::Persistent<v8::Object> persistent;

			static void weakCallback(const v8::WeakCallbackInfo<ObjectWrap> &data) {
				v8::Isolate *isolate = data.GetIsolate();
				v8::HandleScope scope(isolate);
				ObjectWrap *wrap = data.GetParameter();
				assert(wrap->refs == 0);
				wrap->persistent.Reset();
				delete wrap;
			}
	};
}
