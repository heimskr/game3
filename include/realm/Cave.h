#pragma once

#include "realm/Realm.h"

namespace Game3 {
	class Inventory;

	class Cave: public Realm {
		public:
			static Identifier ID() { return {"base", "realm/cave"}; }
			RealmID parentRealm;
			size_t entranceCount = 1;

			Cave(const Cave &) = delete;
			Cave(Cave &&) = delete;
			~Cave() override;

			Cave & operator=(const Cave &) = delete;
			Cave & operator=(Cave &&) = delete;

			bool interactGround(const std::shared_ptr<Player> &, const Position &, Modifiers) override;
			void reveal(const Position &);
			void generateChunk(const ChunkPosition &) override;

			friend class Realm;

		protected:
			using Realm::Realm;

			Cave() = delete;
			Cave(Game &, RealmID, RealmID parent_realm, int seed_);

			void absorbJSON(const nlohmann::json &) override;
			void toJSON(nlohmann::json &) const override;
	};
}
