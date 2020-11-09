#include "scene.hpp"

Scene::Scene() { }
Scene::Scene(glm::vec3 camera_pos, float focal_length) {
	this->camera_view = Maths::translate(camera_pos) * camera_view;
	this->focal_length = focal_length;
}

void Scene::draw(DrawingWindow& window) {
	switch (renderMode) {
	case RenderMode::RASTER:
		depth.assign(window.width * window.height, 0);
		break;
	case RenderMode::RAYTRACED:
		this->_drawRaytraced(window);
		return;
	}

	std::vector<glm::vec3> points{ };
	for (const std::pair<Object, float>& pair : objects) {
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

			switch (renderMode) {
			case RenderMode::WIRE:
				this->_drawWire(window, elem, points);
				break;
			case RenderMode::RASTER:
				this->_drawRaster(window, elem, points, material);
				break;
			default:
				throw std::exception("Unhandled draw mode.");
			}
		}
	}
}

glm::vec3 Scene::_relativePoint(glm::vec3 p) {
	return glm::normalize(glm::vec3(camera_view[0])) * p[0]
		+ glm::normalize(glm::vec3(camera_view[1])) * p[1]
		+ glm::normalize(glm::vec3(camera_view[2])) * p[2];
}
glm::vec3 Scene::_transformPoint(DrawingWindow& w, 
	glm::vec3 p, float scale) {
	auto view{ camera_view * glm::vec4{ p, 1.0f } };
	p = glm::vec3{ view } / view[3];;
	return {
		scale * (focal_length * p[0] / p[2])
			+ (w.width / 2),
		scale * (focal_length * p[1] / p[2])
			+ (w.height / 2),
		1 / p[2]
	};
}
void Scene::_transformPoints(DrawingWindow& w,
	float scale,
	std::vector<glm::vec3> p,
	std::vector<glm::vec3>& out) {
	out.clear();
	for (glm::vec3 point : p) {
		out.push_back(this->_transformPoint(w, point, scale));
	}
}

void Scene::_facePoints(const Face& face,
	const std::vector<glm::vec3> points,
	CanvasPoint& a,
	CanvasPoint& b,
	CanvasPoint& c) {
	auto point_a{ points.at(face.a) };
	auto point_b{ points.at(face.b) };
	auto point_c{ points.at(face.c) };
	a = { point_a[0], point_a[1], point_a[2] };
	b = { point_b[0], point_b[1], point_b[2] };
	c = { point_c[0], point_c[1], point_c[2] };
}
void Scene::_faceTexturePoints(const Face& face,
	const std::vector<glm::vec2> texture_points,
	const TextureMap& map,
	CanvasPoint& a,
	CanvasPoint& b,
	CanvasPoint& c) {
	auto point_ta{ texture_points.at(face.ta) };
	auto point_tb{ texture_points.at(face.tb) };
	auto point_tc{ texture_points.at(face.tc) };
	a.texturePoint = { map.width * point_ta[0],
		map.height * point_ta[1] };
	b.texturePoint = { map.width * point_tb[0],
		map.height * point_tb[1] };
	c.texturePoint = { map.width * point_tc[0],
		map.height * point_tc[1] };
}

bool Scene::_verifyRaytrace(const glm::vec3& v) const {
	if (isinf(v[0]) || isnan(v[0])) return false;
	if (isinf(v[1]) || isnan(v[1])) return false;
	if (isinf(v[2]) || isnan(v[2])) return false;
	if (v[0] <= this->focal_length) return false;
	if (v[1] < 0 || v[1] > 1) return false;
	if (v[2] < 0 || v[2] > 1) return false;
	if (v[1] + v[2] >= 1) return false;
	return true;
}
/*bool Scene::_raytrace(const glm::vec3 ray,
	glm::vec3& output, std::string& output_mtl) const {
	glm::vec3 global_solution{ };
	bool has_intersected{ false };
	auto c{ this->_getCameraPos() };
	for (auto pair : objects) {
		for (const Element& elem : pair.first.getElements()) {
			const std::vector<glm::vec3>& points{ elem.points };
			for (const Face& face : elem.faces) {
				glm::vec3 u{ points.at(face.b) - points.at(face.a) };
				glm::vec3 v{ points.at(face.c) - points.at(face.a) };
				glm::vec3 local_solution{ glm::inverse(
					glm::mat3{ -ray, u, v }) * glm::vec3{ c - points.at(face.a) } };
				if (this->_verifyRaytrace(local_solution)
					&& (!has_intersected
						|| (has_intersected
							&& local_solution[0] < global_solution[0]))) {
					has_intersected = true;
					global_solution = local_solution;
					output = points.at(face.a)
						+ global_solution[1] * u
						+ global_solution[2] * v;
					output_mtl = elem.mtl;
				}
			}
		}
	}
	return has_intersected;
}
*/
void Scene::_drawWire(DrawingWindow& window,
	const Element& elem,
	const std::vector<glm::vec3>& points) {
	CanvasPoint a, b, c;
	for (const Face& face : elem.faces) {
		this->_facePoints(face, points, a, b, c);
		Render::drawTriangle(window, { a, b, c },
			{ 255, 255, 255 }, 255);
	}
}
void Scene::_drawRaster(DrawingWindow& window,
	const Element& elem,
	const std::vector<glm::vec3>& points,
	const Material& material) {
	CanvasPoint a, b, c;
	for (const Face& face : elem.faces) {
		this->_facePoints(face, points, a, b, c);
		switch (material.type) {
		case MaterialType::COLOUR:
			Render::fillTriangle(window, { a, b, c },
				material.colour, 255, &depth);
			break;
		case MaterialType::TEXTURE:
			for (auto pair : textures) {
				if (pair.first.compare(material.texture) != 0) continue;
				this->_faceTexturePoints(face,
					elem.texture_points, pair.second, a, b, c);
				Render::mapTriangle(window,
					{ a, b, c }, pair.second, &depth);
				break;
			}
			break;
		}
	}
}
void Scene::_drawRaytraced(DrawingWindow& window) {
	const float field_of_view{ PI / 2 };
	const float aspect_ratio{ static_cast<float>(window.height) 
		/ static_cast<float>(window.width) };
	const float half_view_width{ glm::tan(field_of_view / 2) };
	const float half_view_height{ half_view_width * aspect_ratio };

	auto c{ this->_getCameraPos() };
	const glm::vec3 delta_x{ this->_relativePoint({ (half_view_width * 2) / window.width, 0, 0 }) };
	const glm::vec3 delta_y{ this->_relativePoint({ 0, (half_view_height * 2) / window.height, 0 }) };
	const glm::vec3 plane_nw{ -1.0f * this->_relativePoint({ half_view_width,
		half_view_height, -focal_length }) };
	for (float x = 0; x < window.width; x++) {
		for (float y = 0; y < window.height; y++) {
			bool collided{ false };
			Face collided_face;
			Element collided_elem;
			float collided_scale{ 0 };
			glm::vec3 global_solution{ }, output{ };
			const glm::vec3 ray{ plane_nw 
				+ delta_x * x
				+ delta_y * y};
			for (const std::pair<Object, float>& pair : objects) {
				for (const Element& elem : pair.first.getElements()) {
					const std::vector<glm::vec3>& points{ elem.points };
					for (const Face& face : elem.faces) {
						glm::vec3 u{ points.at(face.b) - points.at(face.a) };
						glm::vec3 v{ points.at(face.c) - points.at(face.a) };
						auto mat{ glm::inverse(glm::mat3{ -ray, u, v }) };
						auto vec{ glm::vec3{ c - points.at(face.a) } };
						glm::vec3 local_solution{ mat * vec };
						if (this->_verifyRaytrace(local_solution)
							&& (!collided
								|| (collided
									&& local_solution[0] < global_solution[0]))) {
							collided = true;
							global_solution = local_solution;
							output = points.at(face.a)
								+ global_solution[1] * u
								+ global_solution[2] * v;
							collided_scale = pair.second;
							collided_elem = elem;
							collided_face = face;
						}
					}
				}
			}

			if (collided) {
				output = this->_transformPoint(window, output, collided_scale);
				for (auto mtl : materials) {
					if (mtl.name.compare(collided_elem.mtl) != 0) continue;
					window.setPixelColour(
						static_cast<size_t>(output[0]),
						static_cast<size_t>(output[1]),
						Maths::pack(mtl.colour));
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

void Scene::setRenderMode(RenderMode mode) {
	this->renderMode = mode;
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

glm::vec3 Scene::_getCameraPos() const {
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