#include "maths.hpp"

#include <fstream>

glm::vec4 Maths::unpack(uint32_t colour) {
	return {
		(colour >> 16) & 255,
		(colour >> 8) & 255,
		colour & 255,
		colour >> 24
	};
}

uint32_t Maths::pack(float red, float green, float blue, float alpha) {
	uint32_t packed{ static_cast<uint32_t>(alpha) };
	for (float component : { red, green, blue }) {
		packed <<= 8;
		packed += static_cast<uint32_t>(component);
	}
	return packed;
}
uint32_t Maths::pack(glm::vec3 colour, float alpha) {
	return pack(colour[0], colour[1], colour[2], alpha);
}
uint32_t Maths::pack(Colour colour, float alpha) {
	return pack(static_cast<float>(colour.red), 
		static_cast<float>(colour.green), 
		static_cast<float>(colour.blue), alpha);
}

bool Maths::in(float value, float min, float max) {
	if (value < min) return false;
	if (value >= max) return false;
	return true;
}

std::vector<float> Maths::interpolate(float start, float end, size_t numValues) {
	if (numValues == 0) {
		return {};
	} else if (numValues == 1) { 
		return { start };
	} else if (numValues == 2) { return { start, end }; }

	std::vector<float> interpolated{ start };
	const float step{ (end - start) / (numValues - 1) };
	for (size_t index{ 0 }; index < numValues - 2; index++) {
		interpolated.push_back(interpolated.back() + step);
	}
	interpolated.push_back(end);
	return interpolated;
}

std::vector<glm::vec2> Maths::interpolate2D(glm::vec2 start, glm::vec2 end, size_t numValues) {
	if (numValues == 0) {
		return {};
	} else if (numValues == 1) {
		return { start };
	} else if (numValues == 2) { return { start, end }; }

	std::vector<glm::vec2> interpolated{ start };
	glm::vec2 step{ (end - start) };
	step /= (numValues - 1);
	for (size_t index{ 0 }; index < numValues - 2; index++) {
		interpolated.push_back(interpolated.back() + step);
	}
	interpolated.push_back(end);
	return interpolated;
}

std::vector<glm::vec3> Maths::interpolate3D(glm::vec3 start, glm::vec3 end, size_t numValues) {
	if (numValues == 0) {
		return {};
	} else if (numValues == 1) {
		return { start };
	} else if (numValues == 2) { return { start, end }; }

	std::vector<glm::vec3> interpolated{ start };
	glm::vec3 step{ (end - start) };
	step /= (numValues - 1);
	for (size_t index{ 0 }; index < numValues - 2; index++) {
		interpolated.push_back(interpolated.back() + step);
	}
	interpolated.push_back(end);
	return interpolated;
}

glm::mat4 Maths::translate(glm::vec3 v) {
	return {
		glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
		glm::vec4(v, 1.0f)
	};
}
glm::mat4 Maths::rotateX(float angle) {
	return { 
		glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, cos(angle), sin(angle), 0.0f),
		glm::vec4(0.0f, -sin(angle), cos(angle), 0.0f),
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
	};
}

glm::mat4 Maths::rotateY(float angle) {
	return { 
		glm::vec4(cos(angle), 0.0f, -sin(angle), 0.0f),
		glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
		glm::vec4(sin(angle), 0.0f, cos(angle), 0.0f),
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
	};
}

glm::mat4 Maths::rotateZ(float angle) {
	return { 
		glm::vec4(cos(angle), sin(angle), 0.0f, 0.0f),
		glm::vec4(-sin(angle), cos(angle), 0.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
	};
}
