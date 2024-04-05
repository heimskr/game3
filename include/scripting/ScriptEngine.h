#pragma once

#include <v8.h>

#include <atomic>
#include <functional>
#include <map>
#include <optional>
#include <stdexcept>
#include <variant>

namespace Game3 {
	class ScriptEngine {
		public:
			using FunctionAdder = std::function<void(std::function<void(const std::string &, v8::FunctionCallback)>)>;
			using GlobalMutator = std::function<void(v8::Local<v8::ObjectTemplate>)>;

		private:
			GlobalMutator savedMutator{};

		public:
			std::function<void(std::string_view)> onPrint = [](std::string_view text) { std::cout << text; };

			struct Value;
			using FunctionValue = v8::FunctionCallback;
			using ObjectValue = std::map<std::string, Value>;

			struct Value {
				std::variant<v8::Local<v8::Value>, FunctionValue, ObjectValue, std::string, int32_t, double> value;

				template <typename T>
				Value(T &&v):
					value(std::forward<T>(v)) {}
			};

			ScriptEngine();
			ScriptEngine(FunctionAdder);
			ScriptEngine(GlobalMutator);

			std::optional<v8::Local<v8::Value>> execute(const std::string &javascript, bool can_throw = true, const std::function<void(v8::Local<v8::Context>)> &context_mutator = {});
			std::string string(v8::Local<v8::Value>);
			v8::Local<v8::String> string(const std::string &, bool internalized = false);
			v8::Local<v8::Object> object(const ObjectValue &);
			v8::Local<v8::Function> makeValue(const FunctionValue &);
			v8::Local<v8::Function> makeValue(const FunctionValue &, v8::Local<v8::Value> data);
			v8::Local<v8::Object> makeValue(const ObjectValue &);
			v8::Local<v8::String> makeValue(const std::string &);
			v8::Local<v8::Integer> makeValue(int32_t);
			v8::Local<v8::Number> makeValue(double);
			v8::Local<v8::Value> makeValue(const Value &);

			v8::Local<v8::External> wrap(void *);

			void clearContext();
			void print(std::string_view);

			inline v8::Isolate * getIsolate() const { return isolate; }
			inline v8::Local<v8::Context> getContext() const { return globalContext.Get(isolate); }


			static void init(const char *argv0);
			static void deinit();

		private:
			v8::Isolate *isolate = nullptr;
			v8::Global<v8::Context> globalContext;

			void throwException(v8::Isolate *isolate, v8::TryCatch *handler);

			static std::atomic_bool initialized;
			static std::unique_ptr<v8::Platform> platform;
			static v8::Isolate::CreateParams createParams;

			static v8::Isolate * makeIsolate();
			v8::Global<v8::Context> makeContext(GlobalMutator = {});
			v8::Global<v8::Context> makeContext(FunctionAdder);
			static const char * toCString(const v8::String::Utf8Value &);
	};
}
