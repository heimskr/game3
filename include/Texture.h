#pragma once

#include <filesystem>

#include <nlohmann/json.hpp>

#include "registry/Registerable.h"

namespace Game3 {
	class Texture: public NamedRegisterable {
		public:
			unsigned id = 0;
			int width   = 0;
			int height  = 0;
			int format  = 0;
			int filter  = 0;
			bool alpha  = false;
			std::filesystem::path path;
			std::shared_ptr<uint8_t[]> data;

			Texture();
			Texture(Identifier, std::filesystem::path, bool alpha_ = true, int filter_ = -1);

			Texture(const Texture &) = default;
			Texture(Texture &&) = default;

			Texture & operator=(const Texture &) = default;
			Texture & operator=(Texture &&) = default;

			void init();
			void init(std::shared_ptr<uint8_t[]>);
			void bind(int bind_id = -1);
			bool getValid() const { return valid; }

			static std::string filterToString(int);
			static int stringToFilter(const std::string &);

		private:
			bool valid = false;
	};

	std::shared_ptr<Texture> cacheTexture(const std::filesystem::path &, bool alpha = true, int filter = -1);
	std::shared_ptr<Texture> cacheTexture(const char *, bool alpha = true, int filter = -1);
	std::shared_ptr<Texture> cacheTexture(const nlohmann::json &);

	void to_json(nlohmann::json &, const Texture &);
	void from_json(const nlohmann::json &, Texture &);
}
