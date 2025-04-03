#include "util/Demangle.h"
#include "util/Format.h"
#include "util/FS.h"
#include "util/Log.h"
#include "util/Zip.h"

#include <zipios/zipfile.hpp>

#include <fstream>

namespace Game3 {
	Zip::Zip(std::filesystem::path path):
		contents(std::move(path)) {}

	Zip::Zip(std::string contents):
		contents(std::move(contents)) {}

	void Zip::unzipTo(const std::filesystem::path &directory) const & {
		if (std::holds_alternative<std::filesystem::path>(contents)) {
			zipios::ZipFile file(std::get<std::filesystem::path>(contents).string());
			unzipTo(directory, file);
		} else {
			std::stringstream stream(std::get<std::string>(contents));
			zipios::ZipFile file(stream);
			unzipTo(directory, file);
		}
	}

	void Zip::unzipTo(const std::filesystem::path &directory) && {
		if (std::holds_alternative<std::filesystem::path>(contents)) {
			zipios::ZipFile file(std::move(std::get<std::filesystem::path>(contents)).string());
			unzipTo(directory, file);
		} else {
			std::stringstream stream(std::move(std::get<std::string>(contents)));
			zipios::ZipFile file(stream);
			unzipTo(directory, file);
		}
	}

	void Zip::unzipTo(const std::filesystem::path &directory, zipios::ZipFile &zip_file) const {
		std::vector<std::shared_ptr<zipios::FileEntry>> entries = zip_file.entries();

		for (const std::shared_ptr<zipios::FileEntry> &entry: entries) {
			std::filesystem::path full = directory / entry->getName();

			if (!isSubpath(directory, full)) {
				throw std::runtime_error("Refusing to unzip outside the specified root");
			}

			if (entry->isDirectory()) {
				std::filesystem::create_directories(full);
			} else {
				std::shared_ptr<std::istream> input_stream = zip_file.getInputStream(entry->getName());
				std::ofstream output_stream(full, std::ios::binary | std::ios::trunc | std::ios::out);
				output_stream << input_stream->rdbuf();

				auto extra = entry->getExtra();

				INFO("Extra for {}: {}", full, extra);

				// std::filesystem::permissions(entry->
			}
		}
	}
}
