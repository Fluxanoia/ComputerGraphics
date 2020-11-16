#include "scene.hpp"

Scene::Scene() { }
Scene::Scene(glm::vec3 camera_pos, float focal_length) {
	this->camera_pos = camera_pos;
	this->calibration = {
		glm::vec3{ focal_length, 0.0f, 0.0f },
		glm::vec3{ 0.0f, focal_length, 0.0f },
		glm::vec3{ 0.0f, 0.0f, 1.0f }
	};
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

glm::mat4 Scene::_getExtrinsicMatrix() const {
	return {
		glm::vec4{ extrinsic[0], 0.0f },
		glm::vec4{ extrinsic[1], 0.0f },
		glm::vec4{ extrinsic[2], 0.0f },
		glm::vec4{ -1.0f * extrinsic * camera_pos, 1.0f },
	};
}
glm::vec3 Scene::_transformPoint(DrawingWindow& w, 
	glm::vec3 p, float scale) {
	p = calibration * glm::vec3{ this->_getExtrinsicMatrix() * glm::vec4{ p, 1.0f } };
	return glm::vec3{
		-scale * (p[0] / p[2]) + (w.width / 2),
		-scale * (p[1] / p[2]) + (w.height / 2),
		-1 / p[2]
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

bool Scene::_raytrace(const glm::vec3& from,
	const glm::vec3& ray,
	const glm::vec3& a,
	const glm::vec3& b,
	const glm::vec3& c,
	glm::vec3* sr,
	glm::vec3* ur,
	glm::vec3* vr) const {
	glm::vec3 u{ b - a };
	glm::vec3 v{ c - a };
	auto mat{ glm::inverse(glm::mat3{ -ray, u, v }) };
	auto vec{ glm::vec3{ from - a } };
	glm::vec3 s{ mat * vec };
	if (ur != nullptr) (*ur) = u;
	if (vr != nullptr) (*vr) = v;
	if (sr != nullptr) (*sr) = s;

	if (isinf(s[0]) || isnan(s[0])) return false;
	if (isinf(s[1]) || isnan(s[1])) return false;
	if (isinf(s[2]) || isnan(s[2])) return false;
	if (s[0] < 0) return false;
	if (s[1] < 0 || s[1] > 1) return false;
	if (s[2] < 0 || s[2] > 1) return false;
	if (s[1] + s[2] >= 1) return false;
	return true;
}

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
	const float field_of_view{ static_cast<float>(PI / 2) };
	const float aspect_ratio{ static_cast<float>(window.height) 
		/ static_cast<float>(window.width) };
	const float half_view_width{ glm::tan(field_of_view / 2) };
	const float half_view_height{ half_view_width * aspect_ratio };
	const float half_screen_width{ static_cast<float>(window.width / 2) };
	const float half_screen_height{ static_cast<float>(window.height / 2) };
	const float delta_x{ half_view_width / half_screen_width };
	const float delta_y{ half_view_height / half_screen_height };

	for (float x = -half_screen_width; x < half_screen_width; x++) {
		for (float y = -half_screen_height; y < half_screen_height; y++) {
			bool collided{ false };
			Face collided_face;
			Element collided_elem;
			float collided_scale{ 0 };
			glm::vec3 global_solution{ }, output{ };
			glm::vec3 ray{ extrinsic * glm::vec3{ delta_x * x, delta_y * y, -1 } };
			ray[0] *= -1;
			for (const std::pair<Object, float>& pair : objects) {
				for (const Element& elem : pair.first.getElements()) {
					const std::vector<glm::vec3>& points{ elem.points };
					for (const Face& face : elem.faces) {
						glm::vec3 s, u, v;
						if (this->_raytrace(camera_pos, ray, 
								points.at(face.a), points.at(face.b),
								points.at(face.c), &s, &u, &v)
							&& (!collided
								|| (collided
									&& s[0] < global_solution[0]))) {
							collided = true;
							global_solution = s;
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
				ray = output - light;
				auto screen_coords{ this->_transformPoint(window, output, collided_scale) };
				for (auto mtl : materials) {
					if (mtl.name.compare(collided_elem.mtl) != 0) continue;
					if (Render::in(window, glm::vec2{ screen_coords[0], screen_coords[1] })) {
						Colour colour{ mtl.colour };
						this->_darken(colour, 
							this->_brightnessPhong(output,
								collided_elem, collided_face,
								global_solution));
						window.setPixelColour(
							static_cast<size_t>(screen_coords[0]),
							static_cast<size_t>(screen_coords[1]),
							Maths::pack(colour));
					}
				}
			}
		}
	}
}
bool Scene::_occluded(const glm::vec3 v,
	const Element& celem, const Face& cface) {
	for (const std::pair<Object, float>& pair : objects) {
		for (const Element& elem : pair.first.getElements()) {
			const std::vector<glm::vec3>& points{ elem.points };
			for (const Face& face : elem.faces) {
				if (elem.name.compare(celem.name) == 0) {
					if (cface.a == face.a
						&& cface.b == face.b
						&& cface.c == face.c) continue;
				}
				glm::vec3 s;
				if (this->_raytrace(light, v - light,
					points.at(face.a), points.at(face.b),
					points.at(face.c), &s)) {
					if (s[0] > 0.01f && s[0] < 1.0f) return true;
				}
			}
		}
	}
	return false;
}
float Scene::_brightness(const glm::vec3 v,
	const Element& celem, const Face& cface,
	const glm::vec3 normal) {
	auto ray{ v - light };
	auto ray_length{ glm::length(ray) };

	// Specular
	float specular{ std::pow(glm::dot(glm::normalize(camera_pos - v),
		glm::normalize(ray - 2.0f * normal * (glm::dot(ray, normal)))),
		specular_power) };
	if (specular < specular_cull) specular = specular_cull;

	// Angle of Incidence
	float incidence{ -glm::dot(glm::normalize(ray), normal) };
	if (incidence <= 0) incidence = 0;

	// Brightness
	float brightness{ 1.0f };
	if (this->_occluded(v, celem, cface)) {
		brightness = ambient_light;
	} else {
		brightness *= specular;
		brightness *= incidence;
		brightness *= light_strength / (ray_length * ray_length);
	}
	if (brightness > 1.0f) brightness = 1.0f;
	if (brightness < ambient_light) brightness = ambient_light;
	return brightness;
}
float Scene::_brightnessPhong(const glm::vec3 v,
	const Element& celem, const Face& cface,
	const glm::vec3 solution) {
	glm::vec3 na{ 0, 0, 0 };
	glm::vec3 nb{ 0, 0, 0 };
	glm::vec3 nc{ 0, 0, 0 };
	for (const Face& f : celem.faces) {
		for (auto p : { f.a, f.b, f.c }) {
			if (p == cface.a) na += f.normal;
			if (p == cface.b) nb += f.normal;
			if (p == cface.c) nc += f.normal;
		}
	}
	return this->_brightness(v,
		celem, cface, glm::normalize(
			(na * (1 - solution[1] - solution[2]))
			+ (nb * solution[1])
			+ (nc * solution[2])
		));
}
float Scene::_brightnessGouraud(const glm::vec3 v,
	const Element& celem, const Face& cface,
	const glm::vec3 solution) {
	glm::vec3 na{ 0, 0, 0 };
	glm::vec3 nb{ 0, 0, 0 };
	glm::vec3 nc{ 0, 0, 0 };
	for (const Face& f : celem.faces) {
		for (auto p : { f.a, f.b, f.c }) {
			if (p == cface.a) na += f.normal;
			if (p == cface.b) nb += f.normal;
			if (p == cface.c) nc += f.normal;
		}
	}
	float a{ this->_brightness(celem.points.at(cface.a), 
		celem, cface, glm::normalize(na)) };
	float b{ this->_brightness(celem.points.at(cface.b), 
		celem, cface, glm::normalize(nb)) };
	float c{ this->_brightness(celem.points.at(cface.c), 
		celem, cface, glm::normalize(nc)) };
	return (a * (1 - solution[1] - solution[2]))
		+ (b * solution[1])
		+ (c * solution[2]);
}
void Scene::_darken(Colour& c, float f) {
	c.red = static_cast<int>(c.red * f);
	c.green = static_cast<int>(c.green * f);
	c.blue = static_cast<int>(c.blue * f);
}

void Scene::translate(glm::vec3 v) {
	camera_pos += v;
}
void Scene::rotateCamera(glm::vec3 r) {
	extrinsic = Maths::rotateZ(r[2])
		* Maths::rotateY(r[1])
		* Maths::rotateX(r[0])
		* extrinsic;
}
void Scene::rotateWorld(glm::vec3 r) {
	camera_pos = Maths::rotateZ(r[2])
		* Maths::rotateY(r[1])
		* Maths::rotateX(r[0])
		* camera_pos;
}
void Scene::lookAt(glm::vec3 l) {
	glm::vec3 z{ glm::normalize(camera_pos - l) };
	glm::vec3 x{ glm::normalize(glm::cross({ 0.0f, 1.0f, 0.0f }, z)) };
	glm::vec3 y{ glm::normalize(glm::cross(x, z)) };
	extrinsic = glm::inverse(glm::mat3{ x, y, z });
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
