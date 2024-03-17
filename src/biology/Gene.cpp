#include "biology/Gene.h"
#include "net/Buffer.h"
#include "threading/ThreadContext.h"

#include <nlohmann/json.hpp>

#include <format>
#include <random>

namespace Game3 {
	Gene::Gene(std::string name_):
		name(std::move(name_)) {}

	std::unique_ptr<Gene> Gene::fromJSON(const nlohmann::json &json) {
		const std::string type = json.at("type");

		if (type == "float")
			return std::make_unique<FloatGene>(FloatGene::fromJSON(json));

		if (type == "long")
			return std::make_unique<LongGene>(LongGene::fromJSON(json));

		if (type == "circular")
			return std::make_unique<CircularGene>(CircularGene::fromJSON(json));

		throw std::invalid_argument(std::format("Unknown gene type: \"{}\"", type));
	}

	FloatGene::FloatGene(std::string name_, float minimum_, float maximum_, float value_):
		Gene(std::move(name_)), minimum(minimum_), maximum(maximum_), value(clamp(value_)) {}

	FloatGene FloatGene::fromJSON(const nlohmann::json &json) {
		return FloatGene(json.at("name"), json.at("minimum"), json.at("maximum"), json.at("value"));
	}

	void FloatGene::toJSON(nlohmann::json &json) const {
		json["type"] = "float";
		json["name"] = name;
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

	std::string FloatGene::describeShort() const {
		return std::format("{} (float; minimum: {}, maximum: {})", name, minimum, maximum);
	}

	std::vector<std::string> FloatGene::describeLong() const {
		return {
			std::format("Name: {}", name),
			"Type: real",
			std::format("Minimum: {}", minimum),
			std::format("Maximum: {}", maximum),
			std::format("Value: {:.3f}", value),
		};
	}

	void FloatGene::encode(Buffer &buffer) const {
		buffer << name;
		buffer << minimum;
		buffer << maximum;
		buffer << value;
	}

	void FloatGene::decode(Buffer &buffer) {
		buffer >> name;
		buffer >> minimum;
		buffer >> maximum;
		buffer >> value;
	}

	float FloatGene::clamp(float f) const {
		return std::min(maximum, std::max(minimum, f));
	}

	LongGene::LongGene(std::string name_, ValueType minimum_, ValueType maximum_, ValueType value_):
		Gene(std::move(name_)), minimum(minimum_), maximum(maximum_), value(clamp(value_)) {}

	LongGene LongGene::fromJSON(const nlohmann::json &json) {
		return LongGene(json.at("name"), json.at("minimum"), json.at("maximum"), json.at("value"));
	}

	void LongGene::toJSON(nlohmann::json &json) const {
		json["type"] = "long";
		json["name"] = name;
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

	std::string LongGene::describeShort() const {
		return std::format("{} (long; minimum: {}, maximum: {})", name, minimum, maximum);
	}

	std::vector<std::string> LongGene::describeLong() const {
		return {
			std::format("Name: {}", name),
			"Type: integer",
			std::format("Minimum: {}", minimum),
			std::format("Maximum: {}", maximum),
			std::format("Value: {}", value),
		};
	}

	void LongGene::encode(Buffer &buffer) const {
		buffer << name;
		buffer << minimum;
		buffer << maximum;
		buffer << value;
	}

	void LongGene::decode(Buffer &buffer) {
		buffer >> name;
		buffer >> minimum;
		buffer >> maximum;
		buffer >> value;
	}

	auto LongGene::clamp(ValueType v) const -> ValueType {
		return std::min(maximum, std::max(minimum, v));
	}

	CircularGene::CircularGene(std::string name_, float value_):
		Gene(std::move(name_)), value(clamp(value_)) {}

	CircularGene CircularGene::fromJSON(const nlohmann::json &json) {
		return CircularGene(json.at("name"), json.at("value"));
	}

	void CircularGene::toJSON(nlohmann::json &json) const {
		json["type"] = "circular";
		json["name"] = name;
		json["value"] = value;
	}

	void CircularGene::mutate(float strength) {
		const static float base = 1.f / std::sqrt(2.f * M_PIf);
		const float stddev = strength / 6.f;
		const float normal = std::normal_distribution<float>(0.f, stddev)(threadContext.rng);
		value = clamp(value + normal * strength / base);
	}

	std::string CircularGene::describeShort() const {
		return std::format("{} (circular)", name);
	}

	std::vector<std::string> CircularGene::describeLong() const {
		return {
			std::format("Name: {}", name),
			"Type: circular",
			std::format("Value: {:.3f}", value),
		};
	}

	void CircularGene::encode(Buffer &buffer) const {
		buffer << name;
		buffer << value;
	}

	void CircularGene::decode(Buffer &buffer) {
		buffer >> name;
		buffer >> value;
	}

	float CircularGene::clamp(float f) const {
		const float mod = std::fmod(f, 1.f);

		if (f < 0.f) {
			if (mod == 0.f)
				return 0.f;
			return 1.f + mod;
		}

		return mod;
	}

	void to_json(nlohmann::json &json, const Gene &gene) {
		gene.toJSON(json);
	}

	template <>
	std::string Buffer::getType(const FloatGene &) {
		return std::string{'\xe5'};
	}

	Buffer & operator+=(Buffer &buffer, const FloatGene &gene) {
		buffer.appendType(gene);
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
			throw std::invalid_argument("Invalid type (" + hexString(type, true) + ") in buffer (expected e5 for FloatGene)");
		}
		gene.decode(buffer);
		return buffer;
	}

	template <>
	std::string Buffer::getType(const LongGene &) {
		return std::string{'\xe6'};
	}

	Buffer & operator+=(Buffer &buffer, const LongGene &gene) {
		buffer.appendType(gene);
		gene.encode(buffer);
		return buffer;
	}

	Buffer & operator<<(Buffer &buffer, const LongGene &gene) {
		return buffer += gene;
	}

	Buffer & operator>>(Buffer &buffer, LongGene &gene) {
		const auto type = buffer.popType();
		if (!Buffer::typesMatch(type, buffer.getType(gene))) {
			buffer.debug();
			throw std::invalid_argument("Invalid type (" + hexString(type, true) + ") in buffer (expected e6 for LongGene)");
		}
		gene.decode(buffer);
		return buffer;
	}

	template <>
	std::string Buffer::getType(const CircularGene &) {
		return std::string{'\xe7'};
	}

	Buffer & operator+=(Buffer &buffer, const CircularGene &gene) {
		buffer.appendType(gene);
		gene.encode(buffer);
		return buffer;
	}

	Buffer & operator<<(Buffer &buffer, const CircularGene &gene) {
		return buffer += gene;
	}

	Buffer & operator>>(Buffer &buffer, CircularGene &gene) {
		const auto type = buffer.popType();
		if (!Buffer::typesMatch(type, buffer.getType(gene))) {
			buffer.debug();
			throw std::invalid_argument("Invalid type (" + hexString(type, true) + ") in buffer (expected e7 for CircularGene)");
		}
		gene.decode(buffer);
		return buffer;
	}
}
