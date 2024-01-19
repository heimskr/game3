#include "net/Buffer.h"
#include "util/BufferUtil.h"

namespace Game3 {
	#define ADD_BUFFER_METHODS(t) \
		template <> \
		void addToBuffer(Buffer &buffer, t value) { \
			buffer << value; \
		} \
	\
		template <> \
		t getFromBuffer(Buffer &buffer) { \
			return buffer.take<t>(); \
		}

	ADD_BUFFER_METHODS(float)
	ADD_BUFFER_METHODS(double)
	ADD_BUFFER_METHODS(int8_t)
	ADD_BUFFER_METHODS(int16_t)
	ADD_BUFFER_METHODS(int32_t)
	ADD_BUFFER_METHODS(int64_t)
	ADD_BUFFER_METHODS(uint8_t)
	ADD_BUFFER_METHODS(uint16_t)
	ADD_BUFFER_METHODS(uint32_t)
	ADD_BUFFER_METHODS(uint64_t)

	#undef ADD_BUFFER_METHODS
}
