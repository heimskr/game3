#include "threading/ThreadContext.h"

#ifdef __MINGW32__
#include <windows.h>
const DWORD MS_VC_EXCEPTION = 0x406d1388;
#pragma pack(push, 8)
typedef struct tagTHREADNAME_INFO {
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)
void SetThreadName(DWORD dwThreadID, const char* threadName) {
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = threadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;
	try {
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR *) &info);
	} catch (...) {}
}
#endif

namespace Game3 {
	thread_local ThreadContext threadContext;

	void ThreadContext::rename(const char *name) {
#ifdef __MINGW32__
		SetThreadName(-1, name);
#elifdef __APPLE__
		pthread_setname_np(name);
#elifdef __linux__
		pthread_setname_np(pthread_self(), name);
#endif
	}

	void ThreadContext::rename(const std::string &name) {
		rename(name.c_str());
	}
}
