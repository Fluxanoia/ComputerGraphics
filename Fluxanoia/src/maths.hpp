#pragma once

#include <vector>

#include <glm/glm.hpp>

#include <Colour.h>

#define PI 3.1415926535

namespace Maths {

	glm::vec4 unpack(uint32_t colour);

	uint32_t pack(float red, float green, float blue, float alpha = 255);
	uint32_t pack(glm::vec3 colour, float alpha = 255);
	uint32_t pack(Colour colour, float alpha = 255);

	bool in(float value, float min, float max);

	std::vector<float> interpolate(float start, float end, size_t numValues);
	std::vector<glm::vec2> interpolate2D(glm::vec2 start, glm::vec2 end, size_t numValues);
	std::vector<glm::vec3> interpolate3D(glm::vec3 start, glm::vec3 end, size_t numValues);

	glm::mat3 rotateX(float angle);
	glm::mat3 rotateY(float angle);
	glm::mat3 rotateZ(float angle);

};
