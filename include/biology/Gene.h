#pragma once

#include <boost/json/fwd.hpp>

#include <memory>
#include <string>
#include <vector>

namespace Game3 {
	class BasicBuffer;
	class Buffer;

	class Gene {
		public:
			static std::unique_ptr<Gene> fromJSON(const boost::json::value &);

			Gene(std::string name_);

			Gene(const Gene &) = default;
			Gene(Gene &&) noexcept = default;

			virtual ~Gene() = default;

			Gene & operator=(const Gene &) = default;
			Gene & operator=(Gene &&) noexcept = default;

			virtual void toJSON(boost::json::value &) const = 0;
			/** strength ∈ [0.0, 1.0] */
			virtual void mutate(float strength) = 0;
			virtual std::string describeShort() const = 0;
			virtual std::vector<std::string> describeLong() const = 0;
			virtual void encode(Buffer &) const = 0;
			virtual void decode(BasicBuffer &) = 0;

			inline const auto & getName() const { return name; }

		protected:
			std::string name;

			Gene() = default;

		template <typename T>
		friend T makeForBuffer(Buffer &);
	};

	using GenePtr = std::shared_ptr<Gene>;

	class FloatGene: public Gene {
		public:
			static FloatGene fromJSON(const boost::json::value &);

			using Gene::Gene;
			FloatGene(std::string name_, float minimum_, float maximum_, float value_);

			void toJSON(boost::json::value &) const final;
			void mutate(float strength) final;
			std::string describeShort() const final;
			std::vector<std::string> describeLong() const final;
			void encode(Buffer &) const final;
			void decode(BasicBuffer &) final;

			explicit inline operator float() const { return value; }
			inline auto getValue() const { return value; }
			inline void setValue(float new_value) { value = new_value; }

		private:
			float minimum{};
			float maximum{};
			float value{};

			float clamp(float) const;
	};

	class LongGene: public Gene {
		public:
			using ValueType = int64_t;

			static LongGene fromJSON(const boost::json::value &);

			using Gene::Gene;
			LongGene(std::string name_, ValueType minimum_, ValueType maximum_, ValueType value_);

			void toJSON(boost::json::value &) const final;
			void mutate(float strength) final;
			std::string describeShort() const final;
			std::vector<std::string> describeLong() const final;
			void encode(Buffer &) const final;
			void decode(BasicBuffer &) final;

			explicit inline operator ValueType() const { return value; }
			inline auto getValue() const { return value; }
			inline void setValue(ValueType new_value) { value = new_value; }

		private:
			ValueType minimum{};
			ValueType maximum{};
			ValueType value{};

			ValueType clamp(ValueType) const;
	};

	class CircularGene: public Gene {
		public:
			static CircularGene fromJSON(const boost::json::value &);

			using Gene::Gene;
			CircularGene(std::string name_, float value_);

			void toJSON(boost::json::value &) const final;
			void mutate(float strength) final;
			std::string describeShort() const final;
			std::vector<std::string> describeLong() const final;
			void encode(Buffer &) const final;
			void decode(BasicBuffer &) final;

			explicit inline operator float() const { return value; }
			inline auto getValue() const { return value; }
			inline void setValue(float new_value) { value = new_value; }

		private:
			float value{};

			float clamp(float) const;
	};

	class StringGene: public Gene {
		public:
			static StringGene fromJSON(const boost::json::value &);

			using Gene::Gene;
			StringGene(std::string name_, std::string value_);

			void toJSON(boost::json::value &) const final;
			void mutate(float strength) final;
			std::string describeShort() const final;
			std::vector<std::string> describeLong() const final;
			void encode(Buffer &) const final;
			void decode(BasicBuffer &) final;

			explicit inline operator const std::string &() const { return value; }
			inline const auto & getValue() const { return value; }
			inline void setValue(std::string new_value) { value = std::move(new_value); }

		private:
			std::string value{};
	};

	Buffer & operator+=(Buffer &, const FloatGene &);
	Buffer & operator<<(Buffer &, const FloatGene &);
	BasicBuffer & operator>>(BasicBuffer &, FloatGene &);

	Buffer & operator+=(Buffer &, const LongGene &);
	Buffer & operator<<(Buffer &, const LongGene &);
	BasicBuffer & operator>>(BasicBuffer &, LongGene &);

	Buffer & operator+=(Buffer &, const CircularGene &);
	Buffer & operator<<(Buffer &, const CircularGene &);
	BasicBuffer & operator>>(BasicBuffer &, CircularGene &);

	Buffer & operator+=(Buffer &, const StringGene &);
	Buffer & operator<<(Buffer &, const StringGene &);
	BasicBuffer & operator>>(BasicBuffer &, StringGene &);

	void tag_invoke(boost::json::value_from_tag, boost::json::value &, const Gene &);
}
