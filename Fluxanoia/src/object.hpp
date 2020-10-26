#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include <Colour.h>
#include <DrawingWindow.h>
#include <CanvasTriangle.h>

class Object {
private:

	std::vector<Colour> mtls{ };
	void _loadMaterials(const std::string filename);

	struct Tuple {
		size_t a{ 0 }, b{ 0 }, c{ 0 };
	};
	struct Element {
		std::string name{ };
		std::string mtl{ };
		std::vector<glm::vec3> points{ };
		std::vector<Tuple> faces{ };
	};
	std::vector<Element> elements{ };

public:

	void load(const std::string filename, const float scale);

	void draw(DrawingWindow& window, glm::vec3 view, 
		float f, float scale = 1);

};
