#include "biology/Gene.h"
#include "net/Buffer.h"
#include "threading/ThreadContext.h"

#include <nlohmann/json.hpp>

#include <format>
#include <random>

namespace Game3 {
	std::unique_ptr<Gene> Gene::fromJSON(const nlohmann::json &json) {
		const std::string type = json.at("type");

		if (type == "float")
			return std::make_unique<FloatGene>(FloatGene::fromJSON(json));

		if (type == "long")
			return std::make_unique<LongGene>(LongGene::fromJSON(json));

		throw std::invalid_argument(std::format("Unknown gene type: \"{}\"", type));
	}

	std::unique_ptr<Gene> Gene::fromBuffer(Buffer &buffer) {
		GeneType type = buffer.take<GeneType>();
		switch (type) {
			case GeneType::Float: {
				auto gene = std::make_unique<FloatGene>();
				gene->decode(buffer);
				return gene;
			}

			case GeneType::Long: {
				auto gene = std::make_unique<FloatGene>();
				gene->decode(buffer);
				return gene;
			}

			default:
				throw std::invalid_argument(std::format("Unknown gene type: {}", int(type)));
		}
	}

	FloatGene::FloatGene(float minimum_, float maximum_, float value_):
		minimum(minimum_), maximum(maximum_), value(clamp(value_)) {}

	FloatGene FloatGene::fromJSON(const nlohmann::json &json) {
		return FloatGene(json.at("minimum"), json.at("maximum"), json.at("value"));
	}

	void FloatGene::toJSON(nlohmann::json &json) const {
		json["type"] = "float";
		json["minimum"] = minimum;
		json["maximum"] = maximum;
		json["value"] = value;
	}

	void FloatGene::mutate(float strength) {
		const static float base = 1.f / std::sqrt(2.f * M_PIf);
		const float stddev = strength / 6.f;
		const float normal = std::normal_distribution<float>(0.f, stddev)(threadContext.rng);
		value = clamp(value + normal * strength / base);
	}

	void FloatGene::encode(Buffer &buffer) {
		buffer << GeneType::Float;
		buffer << value;
		buffer << minimum;
		buffer << maximum;
	}

	void FloatGene::decode(Buffer &buffer) {
		// It's assumed that the type byte has already been removed.
		buffer >> value;
		buffer >> minimum;
		buffer >> maximum;
	}

	float FloatGene::clamp(float f) const {
		return std::min(maximum, std::max(minimum, f));
	}

	LongGene::LongGene(ValueType minimum_, ValueType maximum_, ValueType value_):
		minimum(minimum_), maximum(maximum_), value(clamp(value_)) {}

	LongGene LongGene::fromJSON(const nlohmann::json &json) {
		return LongGene(json.at("minimum"), json.at("maximum"), json.at("value"));
	}

	void LongGene::toJSON(nlohmann::json &json) const {
		json["type"] = "long";
		json["minimum"] = minimum;
		json["maximum"] = maximum;
		json["value"] = value;
	}

	void LongGene::mutate(float strength) {
		const static float base = 1.f / std::sqrt(2.f * M_PIf);
		const float stddev = strength / 6.f;
		const float normal = std::normal_distribution<float>(0.f, stddev)(threadContext.rng);
		value = clamp(value + ValueType(normal * strength / base * (maximum - minimum) / 2.f));

	}

	void LongGene::encode(Buffer &buffer) {
		buffer << GeneType::Long;
		buffer << value;
		buffer << minimum;
		buffer << maximum;
	}

	void LongGene::decode(Buffer &buffer) {
		// It's assumed that the type byte has already been removed.
		buffer >> value;
		buffer >> minimum;
		buffer >> maximum;
	}

	auto LongGene::clamp(ValueType v) const -> ValueType {
		return std::min(maximum, std::max(minimum, v));
	}

	void to_json(nlohmann::json &json, const Gene &gene) {
		gene.toJSON(json);
	}

	template <>
	std::string Buffer::getType(const FloatGene &) {
		return std::string{'\xe5'};
	}

	Buffer & operator+=(Buffer &buffer, const FloatGene &gene) {
		gene.encode(buffer);
		return buffer;
	}

	Buffer & operator<<(Buffer &buffer, const FloatGene &gene) {
		return buffer += gene;
	}

	Buffer & operator>>(Buffer &buffer, FloatGene &gene) {
		const auto type = buffer.popType();
		if (!Buffer::typesMatch(type, buffer.getType(gene))) {
			buffer.debug();
			throw std::invalid_argument("Invalid type (" + hexString(type, true) + ") in buffer (expected shortlist<i32, 2> for Vector2i)");
		}
		gene.decode(buffer);
		return buffer;
	}
}
