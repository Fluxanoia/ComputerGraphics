#include "object.hpp"

#include <fstream>

#include <Utils.h>

#include "maths.hpp"
#include "render.hpp"

Object::Object(const std::string filename, const float scale) {
	std::ifstream stream{ filename, std::ifstream::binary };
	std::string line;
	
	size_t count{ 0 }, texture_count{ 0 };
	Element element{ };
	while (std::getline(stream, line)) {
		if (line.rfind("mtllib ", 0) == 0) {
			mtllibs.push_back(line.substr(7));
		}
		if (line.rfind("o ", 0) == 0) {
			if (!element.name.empty()) {
				elements.push_back(element);
				count += element.points.size();
				texture_count += element.texture_points.size();
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
		if (line.rfind("vt ", 0) == 0) {
			auto values{ split(line.substr(3), ' ') };
			if (values.size() != 2) throw new std::exception("Non-3D point in .obj file.");
			element.texture_points.push_back({
				std::stof(values[0]),
				std::stof(values[1])
				});
		}
		if (line.rfind("f ", 0) == 0) {
			auto values{ split(line.substr(2), ' ') };
			if (values.size() != 3) throw new std::exception("Non-3D face in .obj file.");
			std::vector<std::pair<std::string, std::string>> pairs{ };
			for (auto v : values) {
				auto pair{ split(v, '/') };
				for (std::string& p : pair) if (p.empty()) p = "0";
				pairs.push_back(std::make_pair(
					pair[0],
					pair[1]
				));
			}
			element.faces.push_back({
				std::stoul(pairs[0].first) - 1 - count,
				std::stoul(pairs[1].first) - 1 - count,
				std::stoul(pairs[2].first) - 1 - count,
				std::stoul(pairs[0].second) - 1 - texture_count,
				std::stoul(pairs[1].second) - 1 - texture_count,
				std::stoul(pairs[2].second) - 1 - texture_count,
			});
		}
	}
	if (!element.name.empty()) elements.push_back(element);
	stream.close();
}

const std::vector<std::string>& Object::getMaterialDependencies() const {
	return mtllibs;
}

const std::vector<Element>& Object::getElements() const {
	return elements;
}