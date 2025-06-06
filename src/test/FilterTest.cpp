#include "game/ClientGame.h"
#include "game/ServerInventory.h"
#include "lib/JSON.h"
#include "pipes/ItemFilter.h"
#include "util/Demangle.h"
#include "util/Timer.h"

namespace Game3 {
	void filterTest() {
		auto game = Game::create(Side::Client, nullptr);

		std::vector<boost::json::value> datas {
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

		std::vector<ItemStackPtr> stacks;

		for (const auto &id: ids) {
			int i = 0;
			for (const auto &data: datas) {
				stacks.push_back(ItemStack::create(game, id, 1, data));
				if (++i % 2 == 0)
					filter.addItem(stacks.back());
			}
		}

		size_t sum = 0;
		size_t total = 0;

		for (size_t i = 0; i < 1'000'000; ++i) {
			for (const ItemStackPtr &stack: stacks) {
				sum += filter.isAllowed(stack, inventory);
				++total;
			}
		}

		std::cout << "Sum: " << sum << " / " << total << '\n';

		timer.stop();
		Timer::summary();
	}
}
