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

	glm::mat4 camera_view{ 
		glm::vec4{ 1.0f, 0.0f, 0.0f, 0.0f },
		glm::vec4{ 0.0f, -1.0f, 0.0f, 0.0f },
		glm::vec4{ 0.0f, 0.0f, -1.0f, 0.0f },
		glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f },
	};
	glm::vec3 _getCameraPos();
	float focal_length{ 1 };
	std::vector<float> depth{ };

	std::vector<std::pair<std::string, TextureMap>> textures{ };
	std::vector<std::pair<Object, float>> objects{ };

	std::vector<Material> materials{ };
	void _loadMaterials(const std::string filename);

	void print(std::string message);

public:

	Scene();
	Scene(glm::vec3 camera_pos, float focal_length);

	void draw(DrawingWindow& window);

	void translate(glm::vec3 v);
	void rotateCamera(glm::vec3 r);
	void rotateWorld(glm::vec3 r);

	void lookAt(glm::vec3 o);

	void loadObject(std::string name, 
		float load_scale, float draw_scale);

};
