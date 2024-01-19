#pragma once

namespace Game3 {
	class Buffer;

	template <typename T>
	void addToBuffer(Buffer &, T);

	template <typename T>
	T getFromBuffer(Buffer &);
}
