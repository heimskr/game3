#pragma once

#include "data/ChunkSet.h"
#include "game/Chunk.h"
#include "math/Concepts.h"
#include "threading/Lockable.h"
#include "types/ChunkPosition.h"
#include "types/Types.h"
#include "util/Math.h"

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include <leveldb/db.h>

namespace Game3 {
	class Buffer;
	class Entity;
	class GameDB;
	class Player;
	class Realm;
	class ServerGame;
	class Tileset;
	class TileEntity;

	struct DBStatus {
		leveldb::Status status;

		DBStatus(leveldb::Status status):
			status(status) {}

		void assertOK();

		inline operator std::string() const {
			return status.ToString();
		}
	};

	struct DBError: std::runtime_error {
		DBStatus status;

		DBError(DBStatus status):
			std::runtime_error(std::string(status)),
			status(status) {}
	};

	struct GameDBScope {
		GameDBScope(GameDB &);
	};

	template <typename T>
	struct RawSlice {
		T data;

		RawSlice(T &&data):
			data(std::forward<T>(data)) {}

		inline operator leveldb::Slice() const {
			return leveldb::Slice(reinterpret_cast<const char *>(&data), sizeof(data));
		}
	};

	template <std::integral T>
	struct RawSlice<T> {
		T data;

		RawSlice(T data):
			data(toLittle(data)) {}

		inline operator leveldb::Slice() const {
			return leveldb::Slice(reinterpret_cast<const char *>(&data), sizeof(data));
		}
	};

	template <std::integral T, size_t S>
	struct RawSlice<std::array<T, S>> {
		std::array<T, S> data;

		RawSlice(const std::array<T, S> &data) {
			for (size_t i = 0; T datum: data) {
				this->data[i] = toLittle(datum);
			}
		}

		inline operator leveldb::Slice() const {
			return leveldb::Slice(reinterpret_cast<const char *>(&data), S * sizeof(T));
		}
	};

	class GameDB {
		private:
			std::weak_ptr<ServerGame> weakGame;
			std::filesystem::path path;

			std::unique_ptr<leveldb::Iterator> getIterator();
			std::unique_ptr<leveldb::Iterator> getStartIterator();

		public:
			Lockable<std::unique_ptr<leveldb::DB>, std::recursive_mutex> database;

			GameDB(const std::shared_ptr<ServerGame> &);

			~GameDB();

			static int64_t getCurrentFormatVersion();
			static std::string getFileExtension();

			void open(std::filesystem::path);
			void close();

			template <typename T = std::string>
			T read(const leveldb::Slice &key);

			template <typename T>
			void readTo(const leveldb::Slice &key, T &out) {
				std::string raw = read(key);
				if (raw.size() != sizeof(T)) {
					throw std::runtime_error(std::format("Invalid size for {}: {} (expected {})", DEMANGLE(T), raw.size(), sizeof(T)));
				}
				std::memcpy(&out, raw.data(), sizeof(T));
			}

			template <Numeric T, size_t S>
			std::array<T, S> readNumbers(const leveldb::Slice &key) {
				using Array = std::array<T, S>;
				std::string raw = read(key);
				if (raw.size() != S * sizeof(T)) {
					throw std::runtime_error(std::format("Invalid size for {}: {} (expected {})", DEMANGLE(Array), raw.size(), sizeof(Array)));
				}
				Array out;
				for (size_t i = 0; i < S; ++i) {
					T little;
					std::memcpy(&little, &raw[sizeof(T) * i], sizeof(T));
					out[i] = toNative(little);
				}
				return out;
			}

			void write(const leveldb::Slice &key, const leveldb::Slice &value);

			template <Numeric T, size_t S>
			void writeNumbers(const leveldb::Slice &key, const std::array<T, S> &numbers) {
				std::string raw(sizeof(T) * S, '\0');
				for (size_t i = 0; T number: numbers) {
					number = toLittle(number);
					std::memcpy(&raw[i], &number, sizeof(T));
					i += sizeof(T);
				}
				write(key, raw);
			}

			void erase(const leveldb::Slice &key);

			/** <  0: this save is too old
			 *  == 0: this save is compatible
			 *  >  0: this save is too new    */
			int64_t getCompatibility();

			void writeAll();
			void readAll();

			void writeMisc();

			void writeRules();
			void readRules();

			/** Writes metadata, chunk data, entity data and tile entity data for all realms. */
			void writeAllRealms();

			void writeRealm(const std::shared_ptr<Realm> &);
			void deleteRealm(std::shared_ptr<Realm>);

			void readVillages();
			void writeVillages();

			void writeChunk(const std::shared_ptr<Realm> &, ChunkPosition);

			void readAllRealms();

			/** Reads metadata from the database and returns an empty realm based on the metadata. */
			std::shared_ptr<Realm> loadRealm(RealmID);

			void writeRealmMeta(const std::shared_ptr<Realm> &);

			std::optional<ChunkSet> getChunk(RealmID, ChunkPosition);

			bool readUser(const std::string &username, std::string *display_name_out, Buffer *buffer_out, std::optional<Place> *release_place);
			void writeUser(const std::string &username, const std::string &display_name, const Buffer &, const std::optional<Place> &release_place);
			void writeUser(Player &);
			void writeReleasePlace(const std::string &username, const std::optional<Place> &release_place);
			bool hasUsername(const std::string &username);
			bool hasName(const std::string &username, const std::string &display_name);
			std::optional<Place> readReleasePlace(const std::string &username);

			void writeTileEntities(const std::function<bool(std::shared_ptr<TileEntity> &)> &);
			void writeTileEntities(const std::shared_ptr<Realm> &);
			void deleteTileEntity(const std::shared_ptr<TileEntity> &);

			void writeEntities(const std::function<bool(std::shared_ptr<Entity> &)> &);
			void writeEntities(const std::shared_ptr<Realm> &);
			void deleteEntity(const std::shared_ptr<Entity> &);

			std::string readRealmTilesetHash(RealmID);
			bool hasTileset(const std::string &hash);

			void writeTilesetMeta(const Tileset &);
			/** Returns whether anything was found. */
			bool readTilesetMeta(const std::string &hash, boost::json::value &);

			bool isOpen();

			inline std::shared_ptr<ServerGame> getGame() const {
				auto game = weakGame.lock();
				assert(game);
				return game;
			}

			template <typename T>
			void writeRaw(const leveldb::Slice &key, const T &value) {
				write(key, RawSlice<T>{value});
			}

			template <Numeric T>
			T readNumber(const leveldb::Slice &key) {
				T out;
				std::string data = read(key);
				if (data.size() != sizeof(T)) {
					throw std::runtime_error(std::format("Invalid size for {}: {} (expected {})", DEMANGLE(T), data.size(), sizeof(T)));
				}
				std::memcpy(&out, data.data(), sizeof(T));
				return toNative(out);
			}

			template <typename C>
			void writeUsers(const C &container) {
				for (const auto &player: container) {
					writeUser(*player);
				}
			}

			template <Returns<void, std::string_view, std::string_view> Fn>
			void iterate(std::string_view prefix, Fn &&function) {
				auto it = getIterator();
				leveldb::Slice slice{prefix.data(), prefix.size()};
				for (it->Seek(slice); it->Valid(); it->Next()) {
					if (!it->key().starts_with(slice)) {
						break;
					}

					function(std::string_view(it->key().data(), it->key().size()), std::string_view(it->value().data(), it->value().size()));
				}

				DBStatus(it->status()).assertOK();
			}

			/** Iterates until the function returns true. Returns whether iteration was canceled. */
			template <Returns<bool, std::string_view, std::string_view> Fn>
			bool iterate(std::string_view prefix, Fn &&function) {
				bool canceled = false;
				auto it = getIterator();
				leveldb::Slice slice{prefix.data(), prefix.size()};
				for (it->Seek(slice); it->Valid(); it->Next()) {
					if (!it->key().starts_with(slice)) {
						break;
					}

					if (function(std::string_view(it->key().data(), it->key().size()), std::string_view(it->value().data(), it->value().size()))) {
						canceled = true;
						break;
					}
				}

				DBStatus(it->status()).assertOK();
				return canceled;
			}

			void write(const leveldb::Slice &key, ChunkPosition);
			void write(const leveldb::Slice &key, Position);
			void write(const leveldb::Slice &key, const Buffer &);
	};
}
