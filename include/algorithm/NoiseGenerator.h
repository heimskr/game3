#pragma once

#include <noise/noise.h>
#include <FastNoise/FastNoise.h>
#include <FastNoise/Generators/Simplex.h>

#include <memory>

namespace Game3 {
	class NoiseGenerator {
		protected:
			int seed = 0;

		public:
			NoiseGenerator() = default;

			NoiseGenerator(int seed_):
				seed(seed_) {}

			virtual ~NoiseGenerator() = default;
			virtual double operator()(double x, double y) const = 0;
			virtual double operator()(double x, double y, double z) const = 0;
			virtual void setSeed(int) = 0;
	};

	class FastNoise2Generator: public NoiseGenerator {
		private:
			FastNoise::SmartNode<FastNoise::OpenSimplex2> fastNoise = FastNoise::New<FastNoise::OpenSimplex2>();
			FastNoise::SmartNode<FastNoise::FractalFBm> fractal = FastNoise::New<FastNoise::FractalFBm>();

		public:
			FastNoise2Generator(): FastNoise2Generator(0) {}

			FastNoise2Generator(int seed_);

			double operator()(double x, double y) const override {
				return fractal->GenSingle2D(x, y, seed);
			}

			double operator()(double x, double y, double z) const override {
				return fractal->GenSingle3D(x, y, z, seed);
			}

			void setSeed(int seed_) override {
				seed = seed_;
			}
	};

	class LibnoiseGenerator: public NoiseGenerator {
		private:
			noise::module::Perlin perlin;

		public:
			using NoiseGenerator::NoiseGenerator;

			LibnoiseGenerator(int seed_): NoiseGenerator(seed_) {
				perlin.SetSeed(seed);
			}

			LibnoiseGenerator(LibnoiseGenerator &&other):
				LibnoiseGenerator(other.seed) {}

			LibnoiseGenerator(const LibnoiseGenerator &other):
				LibnoiseGenerator(other.seed) {}

			LibnoiseGenerator & operator=(LibnoiseGenerator &&other) {
				perlin.SetSeed(other.seed);
				seed = other.seed;
				return *this;
			}

			LibnoiseGenerator & operator=(const LibnoiseGenerator &other) {
				perlin.SetSeed(other.seed);
				seed = other.seed;
				return *this;
			}

			double operator()(double x, double y) const override {
				return perlin.GetValue(x, y, 0.0);
			}

			double operator()(double x, double y, double z) const override {
				return perlin.GetValue(x, y, z);
			}

			void setSeed(int seed_) override {
				seed = seed_;
				perlin.SetSeed(seed_);
			}
	};

	using DefaultNoiseGenerator = FastNoise2Generator;
}
