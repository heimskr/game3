#include "config.h"

#ifdef GAME3_ENABLE_SCRIPTING
#include "Log.h"
#include "game/Game.h"
#include "net/Buffer.h"
#include "scripting/ObjectWrap.h"
#include "scripting/ScriptEngine.h"
#include "scripting/ScriptError.h"
#include "scripting/ScriptUtil.h"
#include "util/JSON.h"
#include "util/Util.h"

#include <libplatform/libplatform.h>
#include <nlohmann/json.hpp>

#include <cassert>
#include <sstream>

namespace Game3 {
	std::atomic_bool ScriptEngine::initialized = false;
	std::unique_ptr<v8::Platform> ScriptEngine::platform;
	v8::Isolate::CreateParams ScriptEngine::createParams;

	ScriptEngine::ScriptEngine(std::shared_ptr<Game> game_):
		game(std::move(game_)),
		isolate(makeIsolate()),
		bufferTemplate(makeBufferTemplate()),
		globalContext(makeContext()) {}

	ScriptEngine::ScriptEngine(std::shared_ptr<Game> game_, FunctionAdder function_adder):
		game(std::move(game_)),
		isolate(makeIsolate()),
		bufferTemplate(makeBufferTemplate()),
		globalContext(makeContext(std::move(function_adder))) {}

	ScriptEngine::ScriptEngine(std::shared_ptr<Game> game_, GlobalMutator global_mutator):
		game(std::move(game_)),
		isolate(makeIsolate()),
		bufferTemplate(makeBufferTemplate()),
		globalContext(makeContext(std::move(global_mutator))) {}

	ScriptEngine::~ScriptEngine() {
		globalContext.Reset();
		bufferTemplate.Reset();
		isolate->Dispose();
	}

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

		if (value->IsBigInt()) {
			v8::Local<v8::BigInt> bigint = value.As<v8::BigInt>();
			if (bigint->WordCount() > 20'000) {
				// Somewhere around this point (2n ** 2000000n will sometimes do it, 2n ** 1500000n won't, but maybe not?),
				// BigInt stringification segfaults on my setup.
				// Something about v8::internal::GetDigits.
				// Let's just return some fake value.
				return "<comically large BigInt>";
			}
		}

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

	nlohmann::json ScriptEngine::getJSON(v8::Local<v8::Value> value) {
		if (value->IsString() || value->IsStringObject())
			return nlohmann::json(string(value));

		if (value->IsNumber() || value->IsNumberObject())
			return nlohmann::json(value->NumberValue(getContext()).ToChecked());

		if (value->IsBigInt())
			return nlohmann::json(value.As<v8::BigInt>()->Int64Value());

		if (value->IsObject()) {
			nlohmann::json out;
			v8::Local<v8::Context> context = getContext();
			v8::Local<v8::Object> obj = value.As<v8::Object>();
			v8::MaybeLocal<v8::Array> maybe_array = obj->GetOwnPropertyNames(context);
			if (maybe_array.IsEmpty())
				throw std::runtime_error("Couldn't get object properties");
			v8::Local<v8::Array> array = maybe_array.ToLocalChecked();

			for (uint32_t i = 0, length = array->Length(); i < length; ++i) {
				v8::Local<v8::Value> key = array->Get(context, i).ToLocalChecked();
				out[string(key)] = getJSON(obj->Get(context, key).ToLocalChecked());
			}

			return out;
		}

		if (value->IsArray()) {
			nlohmann::json out;
			v8::Local<v8::Context> context = getContext();
			v8::Local<v8::Array> array = value.As<v8::Array>();
			for (uint32_t i = 0, length = array->Length(); i < length; ++i)
				out.push_back(getJSON(array->Get(context, i).ToLocalChecked()));
			return out;
		}

		throw std::runtime_error("Couldn't JSONify value");
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
			auto &engine = *reinterpret_cast<ScriptEngine *>(info.Data().As<v8::External>()->Value());
			engine.print(ss.str());
		}, v8::External::New(isolate, this)));

		global->Set(isolate, "gc", v8::FunctionTemplate::New(isolate, [](const v8::FunctionCallbackInfo<v8::Value> &info) {
			auto &engine = *reinterpret_cast<ScriptEngine *>(info.Data().As<v8::External>()->Value());
			auto *isolate = engine.getIsolate();
			int64_t bytes = 100'000'000;
			if (info.Length() == 1 && info[0]->IsNumber())
				bytes = int64_t(info[0].As<v8::Number>()->Value());
			isolate->AdjustAmountOfExternalAllocatedMemory(bytes);
			info.GetReturnValue().Set(v8::BigInt::New(isolate, bytes));
		}, v8::External::New(isolate, this)));

		global->Set(isolate, "Buffer", getBufferTemplate());

		if (global_mutator)
			global_mutator(isolate, global);

		savedMutator = std::move(global_mutator);

		v8::Local<v8::Context> context = v8::Context::New(isolate, nullptr, global);
		return v8::Global<v8::Context>(isolate, context);
	}

	v8::Global<v8::Context> ScriptEngine::makeContext(FunctionAdder function_adder) {
		return makeContext(GlobalMutator([this, function_adder = std::move(function_adder)](v8::Isolate *, v8::Local<v8::ObjectTemplate> global) {
			if (function_adder) {
				function_adder([&](const std::string &name, v8::FunctionCallback function) {
					global->Set(isolate, name.c_str(), v8::FunctionTemplate::New(isolate, function));
				});
			}
		}));
	}

	namespace {
		void toObject(Buffer &buffer, const v8::FunctionCallbackInfo<v8::Value> &info) {
			try {
				nlohmann::json json;

				if (info.Length() >= 1 && info[0]->IsString() && 0 == strcmp("all", *v8::String::Utf8Value(info.GetIsolate(), info[0]))) {
					json = buffer.popAllJSON();
				} else {
					json = buffer.popJSON();
				}

				auto &engine = getExternal<ScriptEngine>(info);

				try {
					if (auto result = engine.execute(stringifyWithBigInt(json)))
						info.GetReturnValue().Set(result.value());
					else
						info.GetReturnValue().SetUndefined();
					return;
				} catch (const std::exception &err) {
					info.GetReturnValue().SetUndefined();
				}
			} catch (const std::exception &err) {
				info.GetIsolate()->ThrowError(v8::String::NewFromUtf8(info.GetIsolate(), err.what()).ToLocalChecked());
			}
		}
	}

	v8::Global<v8::FunctionTemplate> ScriptEngine::makeBufferTemplate() {
		v8::Locker locker(isolate);
		v8::Isolate::Scope isolate_scope(isolate);
		v8::HandleScope handle_scope(isolate);

		v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, [](const v8::FunctionCallbackInfo<v8::Value> &info) {
			auto &engine = getExternal<ScriptEngine>(info);
			v8::Local<v8::Object> this_obj = info.This();
			auto *wrapper = ObjectWrap<Buffer>::make();
			wrapper->wrap(engine.getIsolate(), "Buffer", this_obj);
			wrapper->object->context = engine.game;
		}, wrap(this));

		v8::Local<v8::ObjectTemplate> instance = templ->InstanceTemplate();

		instance->SetInternalFieldCount(2);

		instance->Set(isolate, "add", v8::FunctionTemplate::New(isolate, [](const v8::FunctionCallbackInfo<v8::Value> &info) {
			auto &wrapper = ObjectWrap<Buffer>::unwrap("Buffer", info.This());

			if (info.Length() < 1) {
				info.GetIsolate()->ThrowError("No type specified");
				return;
			}

			std::vector<v8::Local<v8::Value>> values;
			values.reserve(info.Length() - 1);

			for (int i = 1; i < info.Length(); ++i)
				values.push_back(info[i]);

			auto &engine = getExternal<ScriptEngine>(info);
			engine.addToBuffer(*wrapper.object, info[0], values);
			info.GetReturnValue().Set(info.This());
		}, wrap(this)));

		instance->Set(isolate, "debug", v8::FunctionTemplate::New(isolate, [](const v8::FunctionCallbackInfo<v8::Value> &info) {
			auto &wrapper = ObjectWrap<Buffer>::unwrap("Buffer", info.This());
			assert(wrapper.object);
			wrapper->debug();
			info.GetReturnValue().Set(info.This());
		}));

		instance->Set(isolate, "clear", v8::FunctionTemplate::New(isolate, [](const v8::FunctionCallbackInfo<v8::Value> &info) {
			auto &wrapper = ObjectWrap<Buffer>::unwrap("Buffer", info.This());
			assert(wrapper.object);
			wrapper->clear();
			info.GetReturnValue().Set(info.This());
		}));

		instance->Set(isolate, "copy", v8::FunctionTemplate::New(isolate, [](const v8::FunctionCallbackInfo<v8::Value> &info) {
			auto &wrapper = ObjectWrap<Buffer>::unwrap("Buffer", info.This());
			assert(wrapper.object);

			auto &engine = getExternal<ScriptEngine>(info);
			v8::Local<v8::Context> context = engine.getContext();
			v8::Local<v8::Object> new_buffer = engine.getBufferTemplate()->GetFunction(context).ToLocalChecked()->CallAsConstructor(context, 0, nullptr).ToLocalChecked().As<v8::Object>();

			auto &new_wrapper = ObjectWrap<Buffer>::unwrap("Buffer", new_buffer);
			*new_wrapper.object = *wrapper.object;

			info.GetReturnValue().Set(new_buffer);
		}, wrap(this)));

		instance->Set(isolate, "takeObject", v8::FunctionTemplate::New(isolate, [](const v8::FunctionCallbackInfo<v8::Value> &info) {
			auto &wrapper = ObjectWrap<Buffer>::unwrap("Buffer", info.This());
			assert(wrapper.object);
			toObject(*wrapper.object, info);
		}, wrap(this)));

		instance->Set(isolate, "toObject", v8::FunctionTemplate::New(isolate, [](const v8::FunctionCallbackInfo<v8::Value> &info) {
			auto &wrapper = ObjectWrap<Buffer>::unwrap("Buffer", info.This());
			assert(wrapper.object);
			const auto old_skip = wrapper.object->skip;
			toObject(*wrapper.object, info);
			wrapper.object->skip = old_skip;
		}, wrap(this)));

		instance->SetAccessor(string("length"), [](v8::Local<v8::Name>, const v8::PropertyCallbackInfo<v8::Value> &info) {
			auto &wrapper = ObjectWrap<Buffer>::unwrap("Buffer", info.This());
			size_t size = wrapper->size();
			if (size <= UINT32_MAX)
				info.GetReturnValue().Set(uint32_t(size));
			else
				info.GetReturnValue().Set(double(size));
		});

		return v8::Global<v8::FunctionTemplate>(isolate, templ);
	}

	void ScriptEngine::addToBuffer(Buffer &buffer, v8::Local<v8::Value> type_value, std::span<v8::Local<v8::Value>> values, bool in_container) {
		addToBuffer(buffer, describeType(type_value), values, in_container);
	}

	namespace {
		template <typename T>
		void addPrimitive(Buffer &buffer, T value, bool in_container) {
			if (in_container)
				buffer += value;
			else
				buffer << value;
		}
	}

	void ScriptEngine::addToBuffer(Buffer &buffer, const TypeDescription &description, std::span<v8::Local<v8::Value>> values, bool in_container) {
		const auto &[type, primary, secondary] = description;

		if (type == "invalid")
			throw std::invalid_argument("Can't add JS value(s) to buffer: invalid type");

		if (type == "list") {
			if (!in_container)
				buffer.append(getBufferType(description, v8::Undefined(isolate)));
			buffer += uint32_t(values.size());

			TypeDescription subtype = describeType(primary);

			for (auto &value: values)
				addToBuffer(buffer, subtype, std::span<v8::Local<v8::Value>>(&value, 1), true);

			return;
		}

		if (type == "optional") {
			if (values.empty() || (values.size() == 1 && values[0]->IsNullOrUndefined())) {
				buffer.appendType(std::nullopt, false);
				return;
			}

			buffer.append(getBufferType(description, values[0], false));
			addToBuffer(buffer, describeType(primary), values, true);
			return;
		}

		if (type == "map") {
			if (values.size() != 1)
				throw std::invalid_argument("Expected one object");

			buffer.append(getBufferType(description, v8::Undefined(isolate), false));

			v8::Local<v8::Context> context = getContext();
			v8::Local<v8::Object> obj = values[0].As<v8::Object>();
			v8::MaybeLocal<v8::Array> maybe_array = obj->GetOwnPropertyNames(context);
			if (maybe_array.IsEmpty())
				throw std::runtime_error("Couldn't get object properties");
			v8::Local<v8::Array> array = maybe_array.ToLocalChecked();

			TypeDescription key_description = describeType(primary);
			TypeDescription value_description = describeType(secondary);

			buffer += uint32_t(array->Length());

			for (uint32_t i = 0, length = array->Length(); i < length; ++i) {
				v8::Local<v8::Value> key = array->Get(context, i).ToLocalChecked();
				v8::Local<v8::Value> value = obj->Get(context, key).ToLocalChecked();
				addToBuffer(buffer, key_description, std::span<v8::Local<v8::Value>>(&key, 1), true);
				addToBuffer(buffer, value_description, std::span<v8::Local<v8::Value>>(&value, 1), true);
			}

			return;
		}

		if (type == "string") {
			if (values.size() != 1)
				throw std::invalid_argument("Expected one string");

			buffer << string(values[0]);
			return;
		}

		if (values.size() != 1)
			throw std::invalid_argument("Expected one number");

		if (values[0]->IsString() || values[0]->IsStringObject()) {
			std::string str = string(values[0]);
			if (type == "i8")
				addPrimitive(buffer, parseNumber<int8_t>(str), in_container);
			else if (type == "i16")
				addPrimitive(buffer, parseNumber<int16_t>(str), in_container);
			else if (type == "i32")
				addPrimitive(buffer, parseNumber<int32_t>(str), in_container);
			else if (type == "i64")
				addPrimitive(buffer, parseNumber<int64_t>(str), in_container);
			else if (type == "u8")
				addPrimitive(buffer, parseNumber<uint8_t>(str), in_container);
			else if (type == "u16")
				addPrimitive(buffer, parseNumber<uint16_t>(str), in_container);
			else if (type == "u32")
				addPrimitive(buffer, parseNumber<uint32_t>(str), in_container);
			else if (type == "u64")
				addPrimitive(buffer, parseNumber<uint64_t>(str), in_container);
			else if (type == "f32")
				addPrimitive(buffer, parseNumber<float>(str), in_container);
			else if (type == "f64")
				addPrimitive(buffer, parseNumber<double>(str), in_container);
			else
				throw std::invalid_argument(std::format("Unknown type: \"{}\"", type));
			return;
		}

		if (values[0]->IsBigInt() || values[0]->IsBigIntObject()) {
			v8::Local<v8::BigInt> bigint = values[0]->ToBigInt(getContext()).ToLocalChecked();
			if (type == "i8")
				addPrimitive(buffer, int8_t(bigint->Int64Value()), in_container);
			else if (type == "i16")
				addPrimitive(buffer, int16_t(bigint->Int64Value()), in_container);
			else if (type == "i32")
				addPrimitive(buffer, int32_t(bigint->Int64Value()), in_container);
			else if (type == "i64")
				addPrimitive(buffer, bigint->Int64Value(), in_container);
			else if (type == "u8")
				addPrimitive(buffer, uint8_t(bigint->Uint64Value()), in_container);
			else if (type == "u16")
				addPrimitive(buffer, uint16_t(bigint->Uint64Value()), in_container);
			else if (type == "u32")
				addPrimitive(buffer, uint32_t(bigint->Uint64Value()), in_container);
			else if (type == "u64")
				addPrimitive(buffer, bigint->Uint64Value(), in_container);
			else if (type == "f32")
				addPrimitive(buffer, float(bigint->Int64Value()), in_container);
			else if (type == "f64")
				addPrimitive(buffer, double(bigint->Int64Value()), in_container);
			else
				throw std::invalid_argument(std::format("Unknown type: \"{}\"", type));
			return;
		}

		v8::MaybeLocal<v8::Number> maybe_number = values[0]->ToNumber(getContext());
		if (maybe_number.IsEmpty())
			throw std::invalid_argument("Invalid number");

		const double number = maybe_number.ToLocalChecked()->Value();

		if (type == "i8")
			addPrimitive(buffer, int8_t(number), in_container);
		else if (type == "i16")
			addPrimitive(buffer, int16_t(number), in_container);
		else if (type == "i32")
			addPrimitive(buffer, int32_t(number), in_container);
		else if (type == "i64")
			addPrimitive(buffer, int64_t(number), in_container);
		else if (type == "u8")
			addPrimitive(buffer, uint8_t(number), in_container);
		else if (type == "u16")
			addPrimitive(buffer, uint16_t(number), in_container);
		else if (type == "u32")
			addPrimitive(buffer, uint32_t(number), in_container);
		else if (type == "u64")
			addPrimitive(buffer, uint64_t(number), in_container);
		else if (type == "f32")
			addPrimitive(buffer, float(number), in_container);
		else if (type == "f64")
			addPrimitive(buffer, number, in_container);
		else
			throw std::invalid_argument(std::format("Unknown type: \"{}\"", type));
	}

	auto ScriptEngine::describeType(v8::Local<v8::Value> value) -> TypeDescription {
		if (value->IsString()) {
			return {string(value), {}, {}};
		}

		if (value->IsArray()) {
			v8::Local<v8::Array> array = value.As<v8::Array>();
			if (array->Length() < 1)
				return {"invalid", {}, {}};

			v8::Local<v8::Context> context = getContext();
			std::string first = string(array->Get(context, 0).ToLocalChecked());

			if (first == "list" || first == "optional") {
				if (array->Length() == 2)
					return {std::move(first), array->Get(context, 1).ToLocalChecked(), {}};
			} else if (first == "map") {
				if (array->Length() == 3)
					return {"map", array->Get(context, 1).ToLocalChecked(), array->Get(context, 2).ToLocalChecked()};
			}
		}

		return {"invalid", {}, {}};
	}

	std::string ScriptEngine::getBufferType(const TypeDescription &description, v8::Local<v8::Value> value, bool is_subtype) {
		const auto &[type, primary, secondary] = description;

		if (type == "invalid")
			throw std::invalid_argument("Can't stringify invalid type");

		if (type == "i8")
			return Buffer{}.getType(int8_t{}, false);
		if (type == "i16")
			return Buffer{}.getType(int16_t{}, false);
		if (type == "i32")
			return Buffer{}.getType(int32_t{}, false);
		if (type == "i64")
			return Buffer{}.getType(int64_t{}, false);
		if (type == "u8")
			return Buffer{}.getType(uint8_t{}, false);
		if (type == "u16")
			return Buffer{}.getType(uint16_t{}, false);
		if (type == "u32")
			return Buffer{}.getType(uint32_t{}, false);
		if (type == "u64")
			return Buffer{}.getType(uint64_t{}, false);
		if (type == "f32")
			return Buffer{}.getType(float{}, false);
		if (type == "f64")
			return Buffer{}.getType(double{}, false);

		if (type == "string")
			return is_subtype? "\x1f" : Buffer{}.getType(string(value), false);

		if (type == "list")
			return '\x20' + getBufferType(describeType(primary), value, true); // TODO!: check whether using `value` is valid here

		if (type == "optional") {
			if (!is_subtype && value->IsNullOrUndefined())
				return Buffer{}.getType(std::nullopt, false);
			return '\x0b' + getBufferType(describeType(primary), value, true); // TODO!: check whether using `value` is valid here
		}

		if (type == "map")
			return '\x21' + getBufferType(describeType(primary), value, true) + getBufferType(describeType(secondary), value, true); // TODO!: *definitely* check whether using `value` is valid here

		assert(!"Type is anything returned by ScriptEngine::describeType");
	}

	const char * ScriptEngine::toCString(const v8::String::Utf8Value &value) {
		return *value? *value : "<string conversion failed>";
	}
}
#endif
