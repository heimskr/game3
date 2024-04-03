#include "Log.h"
#include "scripting/ScriptEngine.h"

#include <cassert>

namespace Game3 {
	void scriptEngineTest() {
		ScriptEngine engine;

		auto result = engine.execute(R"(
			print(foo.count(1, 2, 3));
			let stuff = [...Array(1000000)].map((x, i) => i);
			print(`Total heap size: ${foo.heap()}`);
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
				}, v8::External::New(engine.getIsolate(), &engine))}
			});

			assert(!context.IsEmpty());
			assert(!context->Global().IsEmpty());

			context->Global()->Set(context, engine.string("foo"), foo).Check();
		});

		INFO("Return value: {}", engine.string(result.value()));
	}
}
