#pragma once

#ifdef GAME3_ENABLE_SCRIPTING
#include <boost/json/fwd.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <v8.h>
#pragma GCC diagnostic pop

#include <atomic>
#include <functional>
#include <map>
#include <optional>
#include <span>
#include <stdexcept>
#include <tuple>
#include <variant>

namespace Game3 {
	class BasicBuffer;
	class Buffer;
	class Game;

	class ScriptEngine {
		public:
			using FunctionAdder = std::function<void(std::function<void(const std::string &, v8::FunctionCallback)>)>;
			using GlobalMutator = std::function<void(v8::Isolate *, v8::Local<v8::ObjectTemplate>)>;

			struct Type {
				virtual ~Type() = default;
				virtual void encode(Buffer &, v8::Local<v8::Value>) = 0;
			};

			struct MapType: Type {

				void encode(Buffer &, v8::Local<v8::Value>) final;
			};

		private:
			GlobalMutator savedMutator{};

		public:
			std::weak_ptr<Game> game;
			std::function<void(std::string_view)> onPrint = [](std::string_view text) { std::cout << text << '\n'; };

			struct Value;
			using FunctionValue = v8::FunctionCallback;
			using ObjectValue = std::map<std::string, Value>;

			struct Value {
				std::variant<v8::Local<v8::Value>, FunctionValue, ObjectValue, std::string, int32_t, double> value;

				template <typename T>
				Value(T &&v):
					value(std::forward<T>(v)) {}
			};

			ScriptEngine(std::shared_ptr<Game>);
			ScriptEngine(std::shared_ptr<Game>, FunctionAdder);
			ScriptEngine(std::shared_ptr<Game>, GlobalMutator);

			~ScriptEngine();

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

			boost::json::value getJSON(v8::Local<v8::Value>);

			void clearContext();
			void print(std::string_view);

			void addToBuffer(Buffer &, v8::Local<v8::Value> type_value, std::span<v8::Local<v8::Value>> values, bool in_container = false);

			inline v8::Isolate * getIsolate() const { return isolate; }
			inline v8::Local<v8::Context> getContext() const { return globalContext.Get(isolate); }
			inline v8::Local<v8::FunctionTemplate> getBufferTemplate() const { return bufferTemplate.Get(isolate); }

			static void init(const char *argv0);
			static void deinit();

		private:
			struct TypeDescription {
				std::string name;
				v8::Local<v8::Value> primary;
				v8::Local<v8::Value> secondary;
			};

			v8::Isolate *isolate = nullptr;
			v8::Global<v8::FunctionTemplate> bufferTemplate;
			v8::Global<v8::Context> globalContext;

			void throwException(v8::Isolate *isolate, v8::TryCatch *handler);
			v8::Global<v8::Context> makeContext(GlobalMutator = {});
			v8::Global<v8::Context> makeContext(FunctionAdder);

			v8::Global<v8::FunctionTemplate> makeBufferTemplate();

			void addToBuffer(Buffer &, const TypeDescription &, std::span<v8::Local<v8::Value>> values, bool in_container = false);
			TypeDescription describeType(v8::Local<v8::Value>);
			std::string getBufferType(const TypeDescription &, v8::Local<v8::Value> value, bool is_subtype = false);

			static std::atomic_bool initialized;
			static std::unique_ptr<v8::Platform> platform;
			static v8::Isolate::CreateParams createParams;

			static v8::Isolate * makeIsolate();
			static const char * toCString(const v8::String::Utf8Value &);
	};
}
#endif
