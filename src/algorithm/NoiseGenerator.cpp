#include "algorithm/NoiseGenerator.h"

namespace Game3 {
	FastNoise2Generator::FastNoise2Generator():
		NoiseGenerator(),
		fastNoise(FastNoise::NewFromEncodedNodeTree(getNodeTree(), FastSIMD::Level_AVX512)) {}

	FastNoise2Generator::FastNoise2Generator(int seed_):
		NoiseGenerator(seed_),
		fastNoise(FastNoise::NewFromEncodedNodeTree(getNodeTree(), FastSIMD::Level_AVX512)) {}

	FastNoise2Generator::FastNoise2Generator(int seed_, const char *node_tree):
		NoiseGenerator(seed_),
		fastNoise(FastNoise::NewFromEncodedNodeTree(node_tree, FastSIMD::Level_AVX512)) {}

	const char * FastNoise2Generator::getNodeTree() {
		// return "EQADAAAAXI9CQBAAAAAAPxoAARkAIQAHAA0ABAAAAAAAAEAJAAAAAAA/AFyPwj8A7FG4PgEgAP//AgAB//8AAACPwnW9AQ4AAwAAAAAAAED//wIAAAAAAD8AAAAAAAAAAABAAAAAgD8AAAAAPwBmZoZA";
		// return "EQACAAAAAAAgQBAAAAAAQBMAw/UoPw0ABAAAAAAAIEAJAABmZiY/AAAAAD8AzcxMPgAzMzM/AAAAAD8=";
		// return "BwA="; // raw Perlin
		// return "DQAFAAAAAAAAQAcAAAAAAD8AAAAAAA==";
		// return "DAABAAAAH4U7QSMAARkADQAEAAAAAAAAQAgAAAAAAD8AAAAAAAENAAUAAAAAAIBA//8AAABcj0I/AEjhej8APQrXPgH//wQA";
		return "GQAZAA0ABAAAAAAAAEAIAAAAAAA/AAAAAAABDQAFAAAAAACAQP//AAAAXI9CPwBI4Xo/AQ0AAwAAAAAAQED//wAAAAAAAD8AAAAAAA==";
	}
}
