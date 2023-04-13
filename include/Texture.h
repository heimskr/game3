#pragma once

#include <filesystem>

#include <nlohmann/json.hpp>

#include "registry/Registerable.h"

namespace Game3 {
	class Texture: public NamedRegisterable {
		public:
			std::shared_ptr<unsigned> id = std::make_shared<unsigned>(0);
			std::shared_ptr<int> width   = std::make_shared<int>(0);
			std::shared_ptr<int> height  = std::make_shared<int>(0);
			std::shared_ptr<int> format  = std::make_shared<int>(0);
			std::shared_ptr<int> filter  = std::make_shared<int>(0);
			std::shared_ptr<bool> alpha  = std::make_shared<bool>(false);
			std::filesystem::path path;
			std::shared_ptr<std::shared_ptr<uint8_t>> data = std::make_shared<std::shared_ptr<uint8_t>>();

			Texture();
			Texture(Identifier, const std::filesystem::path &, bool alpha_ = true, int filter_ = -1);

			Texture(const Texture &) = default;
			Texture(Texture &&) = default;

			Texture & operator=(const Texture &) = default;
			Texture & operator=(Texture &&) = default;

			void init();
			void bind(int bind_id = -1);
			bool valid() const { return *valid_; }

			static std::string filterToString(int);
			static int stringToFilter(const std::string &);

		private:
			std::shared_ptr<bool> valid_ = std::make_shared<bool>(false);
	};

	std::shared_ptr<Texture> cacheTexture(const std::filesystem::path &, bool alpha = true, int filter = -1);
	std::shared_ptr<Texture> cacheTexture(const char *, bool alpha = true, int filter = -1);
	std::shared_ptr<Texture> cacheTexture(const nlohmann::json &);

	void to_json(nlohmann::json &, const Texture &);
	void from_json(const nlohmann::json &, Texture &);
}
