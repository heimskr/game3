#include "game/ClientGame.h"
#include "game/ServerInventory.h"
#include "pipes/ItemFilter.h"
#include "util/Demangle.h"
#include "util/Timer.h"

#include <nlohmann/json.hpp>

namespace Game3 {
	void filterTest() {
		auto game = Game::create(Side::Client, nullptr);

		std::vector<nlohmann::json> datas {
			{},
			{{"formula", ""}},
			{{"formula", "Ca5(PO4)3OH"}},
			{
				{"formula", "Ca5(PO4)3OH"},
				{"nonsense", {
					{"foo", {1.0, 2.0, 3.0, INFINITY}},
					{"bar", {"baz"}},
				}}
			},
		};

		std::vector<Identifier> ids {
			"base:item/coal",
			"base:item/chemical",
			"base:item/grimstone",
		};

		Timer timer{"FilterTest"};

		ServerInventory inventory(nullptr, 30);
		ItemFilter filter(true, true);

		std::vector<ItemStack> stacks;

		for (const auto &id: ids) {
			int i = 0;
			for (const auto &data: datas) {
				stacks.emplace_back(*game, id, 1, data);
				if (++i % 2 == 0)
					filter.addItem(stacks.back());
			}
		}

		size_t sum = 0;
		size_t total = 0;

		for (size_t i = 0; i < 1'000'000; ++i) {
			for (const ItemStack &stack: stacks) {
				sum += filter.isAllowed(stack, inventory);
				++total;
			}
		}

		std::cout << "Sum: " << sum << " / " << total << '\n';

		timer.stop();
		Timer::summary();
	}
}
