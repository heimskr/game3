#ifdef GAME3_ENABLE_SCRIPTING
#include "Log.h"
#include "scripting/ScriptEngine.h"

#include <cassert>
#include <thread>
#include <vector>

namespace Game3 {
	void scriptEngineTest1() {
		ScriptEngine engine(nullptr);

		auto result = engine.execute(R"(
			print(foo.count(1, 2, 3));
			let stuff = [...Array(1000000)].map((x, i) => i);
			print(`Total heap size: ${foo.heap()}`);
			foo.report();
			64;
		)", true, [&engine](v8::Local<v8::Context> context) {
			v8::Local<v8::Object> foo = engine.object({
				{"count", engine.makeValue(+[](const v8::FunctionCallbackInfo<v8::Value> &info) {
					info.GetReturnValue().Set(2 * info.Length());
				})},

				{"heap", engine.makeValue(+[](const v8::FunctionCallbackInfo<v8::Value> &info) {
					auto external = info.Data().As<v8::Value>().As<v8::External>();
					auto &engine = *reinterpret_cast<ScriptEngine *>(external->Value());
					auto isolate = engine.getIsolate();
					v8::HeapStatistics stats;
					isolate->GetHeapStatistics(&stats);
					info.GetReturnValue().Set(int32_t(stats.total_heap_size()));
				}, v8::External::New(engine.getIsolate(), &engine))},

				{"report", engine.makeValue(+[](const v8::FunctionCallbackInfo<v8::Value> &info) {
					double i = info.Length() == 0? -1 : info[0].As<v8::Number>()->Value();
					auto external = info.Data().As<v8::Value>().As<v8::External>();
					auto &engine = *reinterpret_cast<ScriptEngine *>(external->Value());
					auto isolate = engine.getIsolate();
					v8::Locker locker(isolate);
					v8::Isolate::Scope isolate_scope(isolate);
					v8::HeapStatistics stats;
					isolate->GetHeapStatistics(&stats);
					INFO("{}: used {} / limit {}, total {}, available {}", i, stats.used_heap_size(), stats.heap_size_limit(), stats.total_heap_size(), stats.total_available_size());
					info.GetReturnValue().Set(int32_t(stats.total_heap_size()));
				}, v8::External::New(engine.getIsolate(), &engine))},
			});

			assert(!context.IsEmpty());
			assert(!context->Global().IsEmpty());

			context->Global()->Set(context, engine.string("foo"), foo).Check();
		});

		INFO("Return value: {}", engine.string(result.value()));
	}

	/** A demonstration of how not to use V8. */
	void scriptEngineTest2() {
		ScriptEngine engine(nullptr);

		std::vector<std::thread> threads;

		INFO_("Starting threads. Yikes.");

		for (int i = 0; i < 8; ++i) {
			threads.emplace_back([&] {
				auto result = engine.string(engine.execute("{ let x = 0; for (let i = 1; i < 1000000000; ++i) { x += i; }; print(x * 2); x }").value());
				INFO("{}", result);
			});
		}

		for (std::thread &thread: threads)
			thread.join();

		INFO_("Done.");
	}

	void scriptEngineTest() {
		scriptEngineTest1();
	}
}
#else
namespace Game3 {
	void scriptEngineTest() {}
}
#endif
