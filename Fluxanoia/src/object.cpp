#include "object.hpp"

#include <fstream>

#include <Utils.h>

#include "maths.hpp"
#include "render.hpp"

void Object::_loadMaterials(const std::string filename) {
	std::ifstream stream(filename, std::ifstream::binary);
	std::string line;

	Colour colour{ };
	while (std::getline(stream, line)) {
		if (line.rfind("newmtl ", 0) == 0) {
			if (!colour.name.empty()) mtls.push_back(colour);
			colour = Colour{ line.substr(7), 0, 0, 0 };
		}
		if (line.rfind("Kd ", 0) == 0) {
			auto values{ split(line.substr(3), ' ') };
			if (values.size() != 3) throw new std::exception("Invalid colour in .mtl file.");
			colour.red = static_cast<int>(std::stof(values[0]) * 255);
			colour.green = static_cast<int>(std::stof(values[1]) * 255);
			colour.blue = static_cast<int>(std::stof(values[2]) * 255);
		}
	}
	if (!colour.name.empty()) mtls.push_back(colour);

	stream.close();
}

void Object::load(const std::string filename, const float scale) {
	std::ifstream stream{ filename, std::ifstream::binary };
	std::string line;
	
	mtls.clear();
	elements.clear();

	size_t count{ 0 };
	Element element{ };
	while (std::getline(stream, line)) {
		if (line.rfind("mtllib ", 0) == 0) {
			this->_loadMaterials(line.substr(7));
		}
		if (line.rfind("o ", 0) == 0) {
			if (!element.name.empty()) {
				elements.push_back(element);
				count += element.points.size();
			}
			element = Element{ line.substr(2) };
		}
		if (line.rfind("usemtl ", 0) == 0) {
			element.mtl = line.substr(7);
		}
		if (line.rfind("v ", 0) == 0) {
			auto values{ split(line.substr(2), ' ') };
			if (values.size() != 3) throw new std::exception("Non-3D point in .obj file.");
			element.points.push_back({
				std::stof(values[0]) * scale,
				std::stof(values[1]) * scale,
				std::stof(values[2]) * scale
			});
		}
		if (line.rfind("f ", 0) == 0) {
			auto values{ split(line.substr(2), ' ') };
			if (values.size() != 3) throw new std::exception("Non-3D face in .obj file.");
			for (auto v : values) v.pop_back();
			element.faces.push_back({
				std::stoul(values[0]) - 1 - count,
				std::stoul(values[1]) - 1 - count,
				std::stoul(values[2]) - 1 - count
			});
		}
	}
	if (!element.name.empty()) elements.push_back(element);
	stream.close();
}

void Object::draw(DrawingWindow& window, glm::vec3 view, float f, float scale) {
	std::vector<float> depth;
	depth.assign(window.width * window.height, 0);
	for (auto elem : elements) {
		std::vector<glm::vec3> points{ };
		for (auto point : elem.points) {
			const auto dist{ view[2] - point[2] };
			const auto x{ (point[0] - view[0]) / dist };
			const auto y{ (view[1] - point[1]) / dist };
			points.push_back({
				scale * (f * x) + (window.width / 2),
				scale * (f * y) + (window.height / 2),
				1 / dist
			});
		}

		Colour colour{ 255, 255, 255 };
		for (auto mtl : mtls) {
			if (mtl.name.compare(elem.mtl) == 0) {
				colour = mtl;
				break;
			}
		}
		
		for (const Tuple& face : elem.faces) {
			auto point_a{ points.at(face.a) };
			auto point_b{ points.at(face.b) };
			auto point_c{ points.at(face.c) };
			CanvasPoint a{ point_a[0], point_a[1], point_a[2] };
			CanvasPoint b{ point_b[0], point_b[1], point_b[2] };
			CanvasPoint c{ point_c[0], point_c[1], point_c[2] };
			Render::fillTriangle(window, { a, b, c }, colour, 255, &depth);
		}
	}
}
