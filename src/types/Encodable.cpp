#include "net/Buffer.h"
#include "types/Encodable.h"

namespace Game3 {
	Buffer Encodable::encode() {
		Buffer buffer{Side::Invalid};
		encode(buffer);
		return buffer;
	}
}
