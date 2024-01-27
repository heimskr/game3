#include "chemskr/Chemskr.h"
#include "chemskr/Nuclear.h"

#include <cassert>
#include <cstring>
#include <iomanip>
#include <iostream>

void testNuclear();

namespace Game3 {
	bool chemskrTest(int argc, char **argv) {
		if (argc == 2 && strcmp(argv[1], "-n") == 0) {
			testNuclear();
			return true;
		}

		if (strcmp(argv[1], "-c") == 0) {
			const std::string formula = argv[2];

			Chemskr::parser.parse(formula)->debug();

			for (const auto &[element, count]: Chemskr::count(formula))
				std::cout << std::setw(2) << count << " x " << element << std::endl;

			return true;
		}

		if (strcmp(argv[1], "-e") == 0) {
			Chemskr::Equation equation{argv[2]};
			std::cout << "Balanced: " << std::boolalpha << equation.isBalanced() << std::endl;
			std::cout << "Reactants:\n";
			for (const auto &[reactant, count]: equation.getReactants()) {
				if (count == 1)
					std::cout << "- " << reactant << '\n';
				else
					std::cout << "- " << count << '*' << reactant << '\n';
			}
			std::cout << "Products:\n";
			for (const auto &[product, count]: equation.getProducts()) {
				if (count == 1)
					std::cout << "- " << product << '\n';
				else
					std::cout << "- " << count << '*' << product << '\n';
			}

			return true;
		}

		return false;
	}
}

void testNuclear() {
	size_t i = 0;
	for (const auto &[atom, daltons]: std::map(Chemskr::nuclideMasses.begin(), Chemskr::nuclideMasses.end())) {
		std::cout << std::format("{} binding energy: {} MeV\n", atom, atom.calculateBindingEnergy());
		if (++i == 15)
			break;
	}
}
