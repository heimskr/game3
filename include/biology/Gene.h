#pragma once

#include <memory>

#include <nlohmann/json_fwd.hpp>

namespace Game3 {
	class Buffer;

	class Gene {
		public:
			static std::unique_ptr<Gene> fromJSON(const nlohmann::json &);

			Gene() = default;

			virtual void toJSON(nlohmann::json &) const = 0;
			/** strength ∈ [0.0, 1.0] */
			virtual void mutate(float strength) = 0;
			virtual void encode(Buffer &) const = 0;
			virtual void decode(Buffer &) = 0;
	};

	class FloatGene: public Gene {
		public:
			static FloatGene fromJSON(const nlohmann::json &);

			FloatGene() = default;
			FloatGene(float minimum_, float maximum_, float value_);

			void toJSON(nlohmann::json &) const final;
			void mutate(float strength) final;
			void encode(Buffer &) const final;
			void decode(Buffer &) final;

			explicit inline operator float() const { return value; }
			inline auto getValue() const { return value; }

		private:
			float minimum{};
			float maximum{};
			float value{};

			float clamp(float) const;
	};

	class LongGene: public Gene {
		public:
			using ValueType = int64_t;

			static LongGene fromJSON(const nlohmann::json &);

			LongGene() = default;
			LongGene(ValueType minimum_, ValueType maximum_, ValueType value_);

			void toJSON(nlohmann::json &) const final;
			void mutate(float strength) final;
			void encode(Buffer &) const final;
			void decode(Buffer &) final;

			explicit inline operator ValueType() const { return value; }
			inline auto getValue() const { return value; }

		private:
			ValueType minimum{};
			ValueType maximum{};
			ValueType value{};

			ValueType clamp(ValueType) const;
	};

	class CircularGene: public Gene {
		public:
			static CircularGene fromJSON(const nlohmann::json &);

			CircularGene() = default;
			CircularGene(float value_);

			void toJSON(nlohmann::json &) const final;
			void mutate(float strength) final;
			void encode(Buffer &) const final;
			void decode(Buffer &) final;

			explicit inline operator float() const { return value; }
			inline auto getValue() const { return value; }

		private:
			float value{};

			float clamp(float) const;
	};


	Buffer & operator+=(Buffer &, const FloatGene &);
	Buffer & operator<<(Buffer &, const FloatGene &);
	Buffer & operator>>(Buffer &, FloatGene &);

	Buffer & operator+=(Buffer &, const LongGene &);
	Buffer & operator<<(Buffer &, const LongGene &);
	Buffer & operator>>(Buffer &, LongGene &);

	Buffer & operator+=(Buffer &, const CircularGene &);
	Buffer & operator<<(Buffer &, const CircularGene &);
	Buffer & operator>>(Buffer &, CircularGene &);

	void to_json(nlohmann::json &, const Gene &);
}
