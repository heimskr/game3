#pragma once

namespace Game3 {
	class Buffer;

	class Encodable {
		public:
			virtual ~Encodable() = default;
			virtual void encode(Buffer &) = 0;
			virtual void decode(Buffer &) = 0;
			virtual Buffer encode();
	};
}
