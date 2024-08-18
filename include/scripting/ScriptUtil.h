#pragma once

#ifdef GAME3_ENABLE_SCRIPTING
#include <v8.h>

namespace Game3 {
	template <typename T>
	T & getExternal(const v8::FunctionCallbackInfo<v8::Value> &info) {
		return *reinterpret_cast<T *>(info.Data().As<v8::Value>().As<v8::External>()->Value());
	}
}
#endif
