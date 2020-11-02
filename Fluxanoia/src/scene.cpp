#include "scene.hpp"

Scene::Scene() { }
Scene::Scene(glm::vec3 camera_pos, float focal_length) {
	this->camera_pos = glm::vec4{ camera_pos, 1.0f };
	this->focal_length = focal_length;
}

void Scene::draw(DrawingWindow& window) {
	depth.assign(window.width * window.height, 0);

	glm::vec3 camera_pos{
		this->camera_pos[0] / this->camera_pos[3],
		this->camera_pos[1] / this->camera_pos[3],
		this->camera_pos[2] / this->camera_pos[3],
	};

	for (auto pair : objects) {
		for (const Element& elem : pair.first.getElements()) {
			std::vector<glm::vec3> points{ };
			for (glm::vec3 point : elem.points) {

				point = glm::vec3{
					camera_orientation * glm::vec4{
						point[0] - camera_pos[0],
						camera_pos[1] - point[1],
						camera_pos[2] - point[2], 1.0f
					}
				};
				points.push_back({
					pair.second * (focal_length * point[0] / point[2])
						+ (window.width / 2),
					pair.second * (focal_length * point[1] / point[2])
						+ (window.height / 2),
					1 / point[2]
					});
			}

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
	camera_pos = Maths::translate(v) * camera_pos;
}
void Scene::rotate(glm::vec3 r) {
	camera_pos = Maths::rotateX(r[0]) * camera_pos;
	camera_pos = Maths::rotateY(r[1]) * camera_pos;
	camera_pos = Maths::rotateZ(r[2]) * camera_pos;
}
void Scene::orient(glm::vec3 o) {
	camera_orientation *= Maths::rotateX(o[0]);
	camera_orientation *= Maths::rotateY(o[1]);
	camera_orientation *= Maths::rotateZ(o[2]);
}

void Scene::lookAt(glm::vec3 o) {
	glm::vec3 z{ glm::normalize(glm::vec3{
		camera_pos[0] - o[0],
		camera_pos[1] - o[1],
		camera_pos[2] - o[2]
	}) };
	glm::vec3 x{ glm::normalize(glm::cross({ 0.0f, 1.0f, 0.0f }, z)) };
	glm::vec3 y{ glm::normalize(glm::cross(x, z)) };

	camera_orientation = glm::mat4{
		glm::vec4{ x, 0.0f },
		glm::vec4{ y, 0.0f },
		glm::vec4{ z, 0.0f },
		glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f }
	};

	for (int j = 0; j < 4; j++) {
		std::cout << camera_pos[j] << " ";
	}
	std::cout << std::endl << "--------------------" << std::endl;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			std::cout << camera_orientation[j][i] << " ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
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