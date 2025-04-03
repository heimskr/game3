#pragma once

#include <filesystem>
#include <sstream>
#include <variant>
#include <variant>

namespace zipios {
	class ZipFile;
}

namespace Game3 {
	class Zip {
		public:
			Zip(std::filesystem::path);
			Zip(std::string contents);

			void unzipTo(const std::filesystem::path &directory) const &;
			void unzipTo(const std::filesystem::path &directory) &&;

		private:
			std::variant<std::filesystem::path, std::string> contents;

			void unzipTo(const std::filesystem::path &directory, zipios::ZipFile &) const;
	};
}
