#include "scene.hpp"

Scene::Scene() { }
Scene::Scene(glm::vec3 camera_pos, float focal_length) {
	this->camera_view = Maths::translate(camera_pos) * camera_view;
	this->focal_length = focal_length;
}

void Scene::_transformPoints(DrawingWindow& w,
	float scale,
	std::vector<glm::vec3> p,
	std::vector<glm::vec3>& out) {
	out.clear();
	for (glm::vec3 point : p) {
		auto view{ camera_view * glm::vec4{ point, 1.0f } };
		point = glm::vec3{ view } / view[3];

		out.push_back({
			scale * (focal_length * point[0] / point[2])
				+ (w.width / 2),
			scale * (focal_length * point[1] / point[2])
				+ (w.height / 2),
			1 / point[2]
		});
	}
}
void Scene::draw(DrawingWindow& window) {
	depth.assign(window.width * window.height, 0);

	std::vector<glm::vec3> points{ };
	auto camera_pos{ this->_getCameraPos() };
	for (auto pair : objects) {
		for (const Element& elem : pair.first.getElements()) {
			this->_transformPoints(window, pair.second, 
				elem.points, points);

			Material material{ "" };
			for (auto mtl : materials) {
				if (mtl.name.compare(elem.mtl) == 0) {
					material = mtl;
					break;
				}
			}

			for (const Face& face : elem.faces) {
				auto point_a{ points.at(face.a) };
				auto point_b{ points.at(face.b) };
				auto point_c{ points.at(face.c) };
				CanvasPoint a{ point_a[0], point_a[1], point_a[2] };
				CanvasPoint b{ point_b[0], point_b[1], point_b[2] };
				CanvasPoint c{ point_c[0], point_c[1], point_c[2] };
				switch (material.type) {
				case MaterialType::COLOUR:
					Render::fillTriangle(window, { a, b, c },
						material.colour, 255, &depth);
					break;
				case MaterialType::TEXTURE:
					for (auto pair : textures) {
						if (pair.first.compare(material.texture) == 0) {
							auto map{ pair.second };
							auto point_ta{ elem.texture_points.at(face.ta) };
							auto point_tb{ elem.texture_points.at(face.tb) };
							auto point_tc{ elem.texture_points.at(face.tc) };
							a.texturePoint = { map.width * point_ta[0],
								map.height * point_ta[1] };
							b.texturePoint = { map.width * point_tb[0],
								map.height * point_tb[1] };
							c.texturePoint = { map.width * point_tc[0],
								map.height * point_tc[1] };
							Render::mapTriangle(window, { a, b, c }, map, &depth);
						}
					}
					break;
				}
			}
		}
	}
}

void Scene::translate(glm::vec3 v) {
	camera_view = Maths::translate(v) 
		* camera_view;
	print("translating");
}
void Scene::rotateCamera(glm::vec3 r) {
	camera_view = Maths::rotateZ(r[2]) 
		* Maths::rotateY(r[1]) 
		* Maths::rotateX(r[0]) 
		* camera_view;
	print("rotating camera");
}
void Scene::rotateWorld(glm::vec3 r) {
	auto c{ this->_getCameraPos() };
	camera_view = Maths::translate(c)
		* Maths::rotateZ(r[2])
		* Maths::rotateY(r[1])
		* Maths::rotateX(r[0])
		* Maths::translate(-1.0f * c)
		* camera_view;
	print("rotating world");
}

void Scene::print(std::string message) {
#ifdef CG_DEBUG
	std::cout << "---" << message << "---" << std::endl;
	std::cout << "Camera: " << std::endl;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			std::cout << camera_view[j][i] << " ";
		}
		std::cout << std::endl;
	}
	std::cout << "---" << "             " << "---" << std::endl << std::endl;
#endif
}

void Scene::loadObject(std::string name,
	float load_scale, float draw_scale) {
	objects.push_back(std::make_pair(
		Object{ name, load_scale }, draw_scale));
	for (auto mtl_name
		: objects.back().first.getMaterialDependencies()) {
		this->_loadMaterials(mtl_name);
	}
}

glm::vec3 Scene::_getCameraPos() {
	return glm::vec3{ camera_view[3] } / camera_view[3][3];
}

void Scene::_loadMaterials(const std::string filename) {
	std::ifstream stream(filename, std::ifstream::binary);
	std::string line;

	Material mat{ };
	while (std::getline(stream, line)) {
		if (line.rfind("newmtl ", 0) == 0) {
			if (!mat.name.empty()) materials.push_back(mat);
			mat = Material{ line.substr(7) };
		}
		if (line.rfind("Kd ", 0) == 0) {
			auto values{ split(line.substr(3), ' ') };
			if (values.size() != 3) throw new std::exception("Invalid colour in .mtl file.");
			mat.colour.red = static_cast<int>(std::stof(values[0]) * 255);
			mat.colour.green = static_cast<int>(std::stof(values[1]) * 255);
			mat.colour.blue = static_cast<int>(std::stof(values[2]) * 255);
		}
		if (line.rfind("map_Kd ", 0) == 0) {
			mat.type = MaterialType::TEXTURE;
			mat.texture = line.substr(7);
			textures.push_back(std::make_pair(
				mat.texture,
				TextureMap{ mat.texture }
			));
		}
	}
	if (!mat.name.empty()) materials.push_back(mat);

	stream.close();
}