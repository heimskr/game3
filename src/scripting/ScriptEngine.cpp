#include "Log.h"
#include "net/Buffer.h"
#include "scripting/ScriptEngine.h"
#include "scripting/ScriptError.h"
#include "util/Util.h"

#include <libplatform/libplatform.h>

#include <cassert>
#include <sstream>

namespace Game3 {
	std::atomic_bool ScriptEngine::initialized = false;
	std::unique_ptr<v8::Platform> ScriptEngine::platform;
	v8::Isolate::CreateParams ScriptEngine::createParams;

	ScriptEngine::ScriptEngine():
		isolate(makeIsolate()),
		globalContext(makeContext()) {}

	ScriptEngine::ScriptEngine(FunctionAdder function_adder):
		isolate(makeIsolate()),
		globalContext(makeContext(std::move(function_adder))) {}

	ScriptEngine::ScriptEngine(GlobalMutator global_mutator):
		isolate(makeIsolate()),
		globalContext(makeContext(std::move(global_mutator))) {}

	std::optional<v8::Local<v8::Value>> ScriptEngine::execute(const std::string &javascript, bool can_throw, const std::function<void(v8::Local<v8::Context>)> &context_mutator) {
		v8::Locker locker(isolate);
		v8::Isolate::Scope isolate_scope(isolate);
		v8::HandleScope handle_scope(isolate);
		v8::TryCatch try_catch(isolate);
		v8::ScriptOrigin origin(v8::String::NewFromUtf8Literal(isolate, "script"));
		v8::Local<v8::Context> context = globalContext.Get(isolate);
		v8::Context::Scope context_scope(context);

		v8::Local<v8::String> source;
		if (!v8::String::NewFromUtf8(isolate, javascript.c_str()).ToLocal(&source))
			throw std::runtime_error("Couldn't convert javascript from UTF8");

		if (context_mutator)
			context_mutator(context);

		v8::Local<v8::Script> script;
		if (!v8::Script::Compile(context, source, &origin).ToLocal(&script)) {
			if (can_throw)
				throwException(isolate, &try_catch);
			return std::nullopt;
		}

		v8::Local<v8::Value> result;
		if (!script->Run(context).ToLocal(&result)) {
			assert(try_catch.HasCaught());
			if (can_throw)
				throwException(isolate, &try_catch);
			return std::nullopt;
		}

		assert(!try_catch.HasCaught());
		return result;
	}

	std::string ScriptEngine::string(v8::Local<v8::Value> value) {
		v8::Locker locker(isolate);
		v8::Isolate::Scope isolate_scope(isolate);
		v8::HandleScope handle_scope(isolate);
		v8::Context::Scope scope(getContext());
		return toCString(v8::String::Utf8Value(isolate, value));
	}

	v8::Local<v8::String> ScriptEngine::string(const std::string &string, bool internalized) {
		return v8::String::NewFromUtf8(isolate, string.c_str(), internalized? v8::NewStringType::kInternalized : v8::NewStringType::kNormal, string.size()).ToLocalChecked();
	}

	v8::Local<v8::Object> ScriptEngine::object(const ObjectValue &map) {
		v8::Locker locker(isolate);
		v8::Isolate::Scope isolate_scope(isolate);
		v8::EscapableHandleScope handle_scope(isolate);
		v8::Context::Scope context_scope(globalContext.Get(isolate));

		v8::Local<v8::Object> object = v8::Object::New(isolate);

		for (const auto &[key, value]: map) {
			object->Set(isolate->GetCurrentContext(), string(key), makeValue(value)).Check();
		}

		return handle_scope.Escape(object);
	}

	v8::Local<v8::Function> ScriptEngine::makeValue(const FunctionValue &function) {
		v8::Locker locker(isolate);
		v8::Isolate::Scope isolate_scope(isolate);
		return v8::Function::New(globalContext.Get(isolate), function).ToLocalChecked();
	}

	v8::Local<v8::Function> ScriptEngine::makeValue(const FunctionValue &function, v8::Local<v8::Value> data) {
		v8::Locker locker(isolate);
		v8::Isolate::Scope isolate_scope(isolate);
		return v8::Function::New(globalContext.Get(isolate), function, data).ToLocalChecked();
	}

	v8::Local<v8::Object> ScriptEngine::makeValue(const ObjectValue &map) {
		return object(map);
	}

	v8::Local<v8::String> ScriptEngine::makeValue(const std::string &str) {
		return string(str);
	}

	v8::Local<v8::Integer> ScriptEngine::makeValue(int32_t value) {
		v8::Locker locker(isolate);
		v8::Isolate::Scope isolate_scope(isolate);
		return v8::Integer::New(isolate, value);
	}

	v8::Local<v8::Number> ScriptEngine::makeValue(double value) {
		v8::Locker locker(isolate);
		v8::Isolate::Scope isolate_scope(isolate);
		return v8::Number::New(isolate, value);
	}

	v8::Local<v8::Value> ScriptEngine::makeValue(const Value &wrapper) {
		auto &variant = wrapper.value;

		if (std::holds_alternative<FunctionValue>(variant))
			return makeValue(std::get<FunctionValue>(variant));

		if (std::holds_alternative<ObjectValue>(variant))
			return makeValue(std::get<ObjectValue>(variant));

		if (std::holds_alternative<std::string>(variant))
			return makeValue(std::get<std::string>(variant));

		if (std::holds_alternative<int32_t>(variant))
			return makeValue(std::get<int32_t>(variant));

		if (std::holds_alternative<double>(variant))
			return makeValue(std::get<double>(variant));

		if (std::holds_alternative<v8::Local<v8::Value>>(variant))
			return std::get<v8::Local<v8::Value>>(variant);

		throw std::invalid_argument("Invalid variant");
	}

	v8::Local<v8::External> ScriptEngine::wrap(void *item) {
		return v8::External::New(isolate, item);
	}

	void ScriptEngine::addToBuffer(Buffer &buffer, v8::Local<v8::Value> value) {
		v8::Local<v8::Context> context = getContext();

		if (value->IsNumber()) {
			buffer << value->NumberValue(context).ToChecked();
			return;
		}

		if (value->IsString() || value->IsStringObject()) {
			buffer << string(value);
			return;
		}

		if (value->IsArray()) {
			v8::Local<v8::Array> array = value.As<v8::Array>();

			if (array->Length() < 2)
				throw std::invalid_argument("Invalid number of items in array");

			if (array->Length() == 2) {
				std::string first = string(array->Get(context, 0).ToLocalChecked());
				v8::Local<v8::Value> second = array->Get(context, 1).ToLocalChecked();

				if (first == "string") {
					buffer << string(second);
					return;
				}

				if (first == "optional") {
					if (!second->IsArray())
						throw std::invalid_argument("Expected second item of an optional to be an array");

					v8::Local<v8::Array> subarray = second.As<v8::Array>();

					if (subarray->Length() != 2)
						throw std::invalid_argument("Expected second item of an optional to have a length of 2");

					std::string type = string(subarray->Get(context, 0).ToLocalChecked());

					if (type == "optional")
						throw std::invalid_argument("Nested optionals are not supported");

					v8::Local<v8::Value> subsecond = subarray->Get(context, 1).ToLocalChecked();

					if (subsecond->IsNullOrUndefined()) {

					}

				}

				if (first == "i8" || first == "i16" || first == "i32" || first == "i64" || first == "u8" || first == "u16" || first == "u32" || first == "u64" || first == "f32" || first == "f64") {
					v8::MaybeLocal<v8::String> maybe_second_string = second->ToString(context);
					if (!maybe_second_string.IsEmpty()) {
						std::string second_string = string(maybe_second_string.ToLocalChecked());
						if (first == "i8") {
							buffer << parseNumber<int8_t>(second_string);
						} else if (first == "i16") {
							buffer << parseNumber<int16_t>(second_string);
						} else if (first == "i32") {
							buffer << parseNumber<int32_t>(second_string);
						} else if (first == "i64") {
							buffer << parseNumber<int64_t>(second_string);
						} else if (first == "u8") {
							buffer << parseNumber<uint8_t>(second_string);
						} else if (first == "u16") {
							buffer << parseNumber<uint16_t>(second_string);
						} else if (first == "u32") {
							buffer << parseNumber<uint32_t>(second_string);
						} else if (first == "u64") {
							buffer << parseNumber<uint64_t>(second_string);
						} else if (first == "f32") {
							buffer << parseNumber<float>(second_string);
						} else if (first == "f64") {
							buffer << parseNumber<double>(second_string);
						}
						return;
					}

					v8::MaybeLocal<v8::BigInt> maybe_bigint = second->ToBigInt(context);
					if (!maybe_bigint.IsEmpty()) {
						v8::Local<v8::BigInt> bigint = maybe_bigint.ToLocalChecked();
						if (first == "i8") {
							buffer << int8_t(bigint->Int64Value());
						} else if (first == "i16") {
							buffer << int16_t(bigint->Int64Value());
						} else if (first == "i32") {
							buffer << int32_t(bigint->Int64Value());
						} else if (first == "i64") {
							buffer << int64_t(bigint->Int64Value());
						} else if (first == "u8") {
							buffer << uint8_t(bigint->Uint64Value());
						} else if (first == "u16") {
							buffer << uint16_t(bigint->Uint64Value());
						} else if (first == "u32") {
							buffer << uint32_t(bigint->Uint64Value());
						} else if (first == "u64") {
							buffer << uint64_t(bigint->Uint64Value());
						} else if (first == "f32") {
							buffer << float(bigint->Uint64Value());
						} else if (first == "f64") {
							buffer << double(bigint->Uint64Value());
						}
						return;
					}

					v8::MaybeLocal<v8::Number> maybe_number = second->ToNumber(context);
					if (maybe_number.IsEmpty())
						throw std::runtime_error("Invalid " + first);
					v8::Local<v8::Number> number = maybe_number.ToLocalChecked();

					if (first == "i8") {
						buffer << int8_t(number->Value());
					} else if (first == "i16") {
						buffer << int16_t(number->Value());
					} else if (first == "i32") {
						buffer << int32_t(number->Value());
					} else if (first == "i64") {
						buffer << int64_t(number->Value());
					} else if (first == "u8") {
						buffer << uint8_t(number->Value());
					} else if (first == "u16") {
						buffer << uint16_t(number->Value());
					} else if (first == "u32") {
						buffer << uint32_t(number->Value());
					} else if (first == "u64") {
						buffer << uint64_t(number->Value());
					} else if (first == "f32") {
						buffer << float(number->Value());
					} else if (first == "f64") {
						buffer << double(number->Value());
					}

					return;
				}
			}
		}
	}

	void ScriptEngine::clearContext() {
		globalContext = makeContext(std::move(savedMutator));
	}

	void ScriptEngine::print(std::string_view text) {
		if (onPrint)
			onPrint(text);
	}

	void ScriptEngine::init(const char *argv0) {
		if (initialized.exchange(true))
			return;
		v8::V8::InitializeICUDefaultLocation(argv0);
		v8::V8::InitializeExternalStartupData(argv0);
		platform = v8::platform::NewDefaultPlatform();
		v8::V8::InitializePlatform(platform.get());
		v8::V8::Initialize();
		createParams.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
		createParams.oom_error_callback = v8::OOMErrorCallback([](const char *location, const v8::OOMDetails &details) {
			INFO("OOM @ {}: {} (is heap OOM: {})", location, details.detail, details.is_heap_oom);
		});
	}

	void ScriptEngine::deinit() {
		if (initialized)
			return;
		// Somewhat data racey.
		v8::V8::Dispose();
		v8::V8::DisposePlatform();
		delete createParams.array_buffer_allocator;
		initialized = false;
	}

	void ScriptEngine::throwException(v8::Isolate *isolate, v8::TryCatch *try_catch) {
		v8::Locker locker(isolate);
		v8::Isolate::Scope isolate_scope(isolate);
		v8::HandleScope handle_scope(isolate);
		v8::String::Utf8Value exception(isolate, try_catch->Exception());
		const char *exception_string = toCString(exception);
		v8::Local<v8::Message> message = try_catch->Message();

		if (message.IsEmpty())
			throw ScriptError(exception_string);

		v8::Local<v8::Context> context(isolate->GetCurrentContext());
		const int line   = message->GetLineNumber(context).FromJust();
		const int column = message->GetStartColumn(context).FromJust();
		throw ScriptError(exception_string, line, column);
	}

	v8::Isolate * ScriptEngine::makeIsolate() {
		assert(initialized);
		return v8::Isolate::New(createParams);
	}

	v8::Global<v8::Context> ScriptEngine::makeContext(GlobalMutator global_mutator) {
		v8::Locker locker(isolate);
		v8::Isolate::Scope isolate_scope(isolate);
		v8::HandleScope handle_scope(isolate);
		v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);

		global->Set(isolate, "print", v8::FunctionTemplate::New(isolate, [](const v8::FunctionCallbackInfo<v8::Value> &info) {
			bool first = true;
			std::stringstream ss;
			for (int i = 0; i < info.Length(); ++i) {
				v8::HandleScope handle_scope(info.GetIsolate());
				if (first)
					first = false;
				else
					ss << ' ';
				ss << toCString(v8::String::Utf8Value(info.GetIsolate(), info[i]));
			}
			auto &engine = *reinterpret_cast<ScriptEngine *>(info.Data().As<v8::Value>().As<v8::External>()->Value());
			engine.print(ss.str());
		}, v8::External::New(isolate, this)));

		if (global_mutator)
			global_mutator(global);

		savedMutator = std::move(global_mutator);

		v8::Local<v8::Context> context = v8::Context::New(isolate, nullptr, global);
		return v8::Global<v8::Context>(isolate, context);
	}

	v8::Global<v8::Context> ScriptEngine::makeContext(FunctionAdder function_adder) {
		return makeContext(GlobalMutator([this, function_adder = std::move(function_adder)](v8::Local<v8::ObjectTemplate> global) {
			if (function_adder) {
				function_adder([&](const std::string &name, v8::FunctionCallback function) {
					global->Set(isolate, name.c_str(), v8::FunctionTemplate::New(isolate, function));
				});
			}
		}));
	}

	const char * ScriptEngine::toCString(const v8::String::Utf8Value &value) {
		return *value? *value : "<string conversion failed>";
	}
}
