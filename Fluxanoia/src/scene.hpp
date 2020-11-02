#pragma once

#include <vector>

#include <glm/glm.hpp>

#include <TextureMap.h>
#include <DrawingWindow.h>

#include "render.hpp"
#include "object.hpp"

class Scene {
private:

	enum class MaterialType { COLOUR, TEXTURE };
	struct Material {
		std::string name;
		MaterialType type{ MaterialType::COLOUR };

		Colour colour{ 0, 0, 0 };
		std::string texture{ };
	};

	glm::vec4 camera_pos{ 0, 0, 0, 0 };
	glm::mat4 camera_orientation{ 
		glm::vec4{ 1.0f, 0.0f, 0.0f, 0.0f },
		glm::vec4{ 0.0f, 1.0f, 0.0f, 0.0f },
		glm::vec4{ 0.0f, 0.0f, 1.0f, 0.0f },
		glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f },
	};
	float focal_length{ 1 };
	std::vector<float> depth{ };

	std::vector<std::pair<std::string, TextureMap>> textures{ };
	std::vector<std::pair<Object, float>> objects{ };

	std::vector<Material> materials{ };
	void _loadMaterials(const std::string filename);

public:

	Scene();
	Scene(glm::vec3 camera_pos, float focal_length);

	void draw(DrawingWindow& window);

	void translate(glm::vec3 v);
	void rotate(glm::vec3 r);
	void orient(glm::vec3 o);

	void lookAt(glm::vec3 o);

	void loadObject(std::string name, 
		float load_scale, float draw_scale);

};
