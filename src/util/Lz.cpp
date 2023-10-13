#include "util/Lz.h"
#include "util/Util.h"

#include <limits>
#include <memory>
#include <string>

#include <lz4.h>

namespace LZ4 {
	using Game3::safeCast;

	std::vector<uint8_t> compress(std::span<const uint8_t> input) {
		if (static_cast<size_t>(std::numeric_limits<int>::max()) < input.size_bytes())
			throw std::invalid_argument("Can't compress: input size is too large");

		std::vector<uint8_t> output;
		output.resize(LZ4_compressBound(static_cast<int>(input.size())));

		const int result = LZ4_compress_default(reinterpret_cast<const char *>(input.data()), reinterpret_cast<char *>(output.data()), safeCast<int>(input.size_bytes()), safeCast<int>(output.size()));
		if (result == 0)
			throw std::runtime_error("Failed to compress");

		output.resize(result);
		return output;
	}

	std::vector<uint8_t> decompress(std::span<const uint8_t> input) {
		if (static_cast<size_t>(std::numeric_limits<int>::max()) < input.size())
			throw std::invalid_argument("Can't decompress: input size is too large");

		std::vector<uint8_t> output;
		output.resize(input.size() * 4);

		const char *input_data = reinterpret_cast<const char *>(input.data());
		const int input_size = safeCast<int>(input.size_bytes());

		for (;;) {
			const int result = LZ4_decompress_safe(input_data, reinterpret_cast<char *>(output.data()), input_size, safeCast<int>(output.size()));

			if (result == -1)
				throw std::invalid_argument("Can't decompress: input is malformed");

			if (result < -1) {
				output.resize(output.size() * 4);
				continue;
			}

			output.resize(result);
			return output;
		}
	}
}
