#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include <Colour.h>
#include <DrawingWindow.h>
#include <CanvasTriangle.h>

struct Face {
	size_t a{ 0 }, b{ 0 }, c{ 0 };
	size_t ta{ 0 }, tb{ 0 }, tc{ 0 };
	glm::vec3 normal;
};
struct Element {
	std::string name{ };
	std::string mtl{ };
	std::vector<glm::vec3> points{ };
	std::vector<glm::vec2> texture_points{ };
	std::vector<Face> faces{ };
};

class Object {
private:

	std::vector<std::string> mtllibs{ };
	std::vector<Element> elements{ };

public:

	Object(const std::string filename, const float load_scale);

	const std::vector<std::string>& getMaterialDependencies() const;
	const std::vector<Element>& getElements() const;

};
