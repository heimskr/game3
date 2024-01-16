#include "algorithm/NoiseGenerator.h"

namespace Game3 {
	FastNoise2Generator::FastNoise2Generator(int seed_): NoiseGenerator(seed_) {
		fractal->SetSource(fastNoise);
		fractal->SetOctaveCount(5);
		fractal->SetLacunarity(2.0);
	}
}
