#pragma once

namespace Game3 {
	class BasicBuffer;
	class Buffer;

	class Encodable {
		public:
			virtual ~Encodable() = default;
			virtual void encode(Buffer &) = 0;
			virtual void decode(BasicBuffer &) = 0;
			virtual Buffer encode();
	};
}
