#include <bit>
#include <memory>

#include <zstd.h>

#include "util/Zstd.h"

namespace Game3 {
	std::vector<uint8_t> decompress8(std::span<const uint8_t> span) {
		const size_t out_size = ZSTD_DStreamOutSize();
		std::vector<uint8_t> out_buffer(out_size);
		std::vector<uint8_t> out;

		auto context = std::unique_ptr<ZSTD_DCtx, size_t(*)(ZSTD_DCtx *)>(ZSTD_createDCtx(), ZSTD_freeDCtx);
		auto stream  = std::unique_ptr<ZSTD_DStream, size_t(*)(ZSTD_DStream *)>(ZSTD_createDStream(), ZSTD_freeDStream);

		ZSTD_inBuffer input {span.data(), span.size_bytes(), 0};

		size_t last_result = 0;

		while (input.pos < input.size) {
			ZSTD_outBuffer output {&out_buffer[0], out_size, 0};
			const size_t result = ZSTD_decompressStream(stream.get(), &output, &input);
			if (ZSTD_isError(result))
				throw std::runtime_error("Couldn't decompress tiles");
			last_result = result;

			const uint8_t *raw_bytes = out_buffer.data();
			out.insert(out.end(), raw_bytes, raw_bytes + output.pos / sizeof(uint8_t));
		}

		if (last_result != 0)
			throw std::runtime_error("Reached end of tile input without finishing decompression");

		return out;
	}

	std::vector<uint16_t> decompress16(std::span<const uint8_t> span) {
		std::vector<uint8_t> decompressed = decompress8(span);
		std::vector<uint16_t> out;
		out.reserve(decompressed.size() / 2);

		if constexpr (std::endian::native == std::endian::little) {
			for (size_t i = 0, max = decompressed.size() / 2; i < max; i += 2)
				out.emplace_back(*reinterpret_cast<const uint16_t *>(&decompressed[i]));
		} else {
			for (size_t i = 0, max = decompressed.size() / 2; i < max; i += 2)
				out.emplace_back(decompressed[i] | (static_cast<uint16_t>(decompressed[i + 1]) << 8));
		}

		return out;
	}

	std::vector<uint32_t> decompress32(std::span<const uint8_t> span) {
		std::vector<uint8_t> decompressed = decompress8(span);
		std::vector<uint32_t> out;
		out.reserve(decompressed.size() / 2);

		if constexpr (std::endian::native == std::endian::little) {
			for (size_t i = 0, max = decompressed.size() / 2; i < max; i += 2)
				out.emplace_back(*reinterpret_cast<const uint32_t *>(&decompressed[i]));
		} else {
			for (size_t i = 0, max = decompressed.size() / 2; i < max; i += 2)
				out.emplace_back(decompressed[i] | (static_cast<uint32_t>(decompressed[i + 1]) <<  8)
				                                 | (static_cast<uint32_t>(decompressed[i + 2]) << 16)
				                                 | (static_cast<uint32_t>(decompressed[i + 3]) << 24));
		}

		return out;
	}

	std::vector<uint8_t> compress(std::span<const uint8_t> span) {
		const auto buffer_size = ZSTD_compressBound(span.size_bytes());
		auto buffer = std::vector<uint8_t>(buffer_size);
		auto result = ZSTD_compress(&buffer[0], buffer_size, span.data(), span.size_bytes(), ZSTD_maxCLevel());
		if (ZSTD_isError(result))
			throw std::runtime_error("Couldn't compress data");
		buffer.resize(result);
		return buffer;
	}

	std::vector<uint8_t> compress(std::span<const uint16_t> span) {
		if (std::endian::native == std::endian::little)
			return compress(std::span<const uint8_t>(reinterpret_cast<const uint8_t *>(span.data()), span.size_bytes()));

		std::vector<uint8_t> bytes;
		bytes.reserve(span.size_bytes());

		for (const auto item: span) {
			bytes.emplace_back(item & 0xff);
			bytes.emplace_back((item >> 8) & 0xff);
		}

		return compress(std::span<const uint8_t>(bytes.data(), bytes.size()));
	}

	std::vector<uint8_t> compress(std::span<const uint32_t> span) {
		if (std::endian::native == std::endian::little)
			return compress(std::span<const uint8_t>(reinterpret_cast<const uint8_t *>(span.data()), span.size_bytes()));

		std::vector<uint8_t> bytes;
		bytes.reserve(span.size_bytes());

		for (const auto item: span) {
			bytes.emplace_back(item & 0xff);
			bytes.emplace_back((item >>  8) & 0xff);
			bytes.emplace_back((item >> 16) & 0xff);
			bytes.emplace_back((item >> 24) & 0xff);
		}

		return compress(std::span<const uint8_t>(bytes.data(), bytes.size()));
	}
}
