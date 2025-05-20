#pragma once

#include "item/Plantable.h"

namespace Game3 {
	class Sapling: public Plantable {
		public:
			using Plantable::Plantable;
			bool use(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>) override;
			bool drag(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>, DragAction) override;
			bool plant(InventoryPtr, Slot, const ItemStackPtr &, const Place &) override;

			virtual Identifier getRealmType() const { return "base:realm/overworld"; }
			virtual Identifier getSoilCategory() const { return "base:category/plant_soil"; }
			virtual std::vector<Identifier> getTreeTypes() const = 0;
	};

	class GrasslandSapling: public Sapling {
		public:
			using Sapling::Sapling;

			std::vector<Identifier> getTreeTypes() const final {
				return {
					"base:tile/tree1_stump", "base:tile/tree2_stump", "base:tile/tree3_stump",
					"base:tile/tree1_honey_stump", "base:tile/tree2_honey_stump", "base:tile/tree3_honey_stump",
				};
			}
	};

	class SnowySapling: public Sapling {
		public:
			using Sapling::Sapling;

			std::vector<Identifier> getTreeTypes() const final {
				return {"base:tile/winter_tree1_stump", "base:tile/winter_tree2_stump", "base:tile/winter_tree3_stump"};
			}
	};

	class DesertSapling: public Sapling {
		public:
			using Sapling::Sapling;

			virtual Identifier getSoilCategory() const { return "base:category/cactus_soil"; }
			std::vector<Identifier> getTreeTypes() const final {
				return {"base:tile/cactus1_young", "base:tile/cactus2_young", "base:tile/cactus3_young", "base:tile/cactus4_young"};
			}
	};
}
