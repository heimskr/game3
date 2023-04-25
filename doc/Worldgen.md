# World Generation

Every tile in an overworld realm has a biome associated with it. Biomes are assigned using a map of Perlin noise.

At the beginning of overworld generation, biomes are assigned to each tile.

Then, pregeneration begins by spawning a number of threads equal to the number of *n* x *n* regions (*n* = 128 by default). Each thread loops over each tile and calls the corresponding biome's `generate` method for that tile. Then it computes a vector of locations where ore deposits can spawn. The vector is shuffled and for each ore type, the last *s*/1000 positions, where *s* is the size of the vector at the start of each ore type's generation, are given a possibility to spawn an ore deposit. Whether an ore deposit spawns is based on the Perlin noise sampled in the biome's `generate` method.

Once all those threads have finished, the overworld generation code collects in a vector all the tiles where a town can spawn. It launches a number of threads to check for rectangular regions large enough to hold a town. Once all the threads have finished and there is at least one candidate, one of the candidates is chosen at random and a town is generated there.

Next is postgeneration. A number of threads is spawned (same number as in pregeneration) that each govern a region of the realm. Each thread loops over every tile in its region and calls the corresponding biome's `postgen` method for that tile.

Finally, once all the postgeneration threads have finished, the pathmap is made and overworld generation is complete.
