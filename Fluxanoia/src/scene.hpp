#pragma once

#include <vector>

#include <glm/glm.hpp>

#include <TextureMap.h>
#include <DrawingWindow.h>

#include "render.hpp"
#include "object.hpp"

class Scene {
private:

	RenderMode renderMode{ RenderMode::WIRE };

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
	glm::vec3 _getCameraPos() const;
	float focal_length{ 1 };
	std::vector<float> depth{ };

	std::vector<std::pair<std::string, TextureMap>> textures{ };
	std::vector<std::pair<Object, float>> objects{ };
	
	glm::vec3 _relativePoint(glm::vec3 p);
	glm::vec3 _transformPoint(DrawingWindow& w, 
		glm::vec3 p, float scale);
	void _transformPoints(DrawingWindow& w,
		float scale,
		std::vector<glm::vec3> p,
		std::vector<glm::vec3>& out);

	std::vector<Material> materials{ };
	void _loadMaterials(const std::string filename);

	void _facePoints(const Face& face,
		const std::vector<glm::vec3> points,
		CanvasPoint& a,
		CanvasPoint& b,
		CanvasPoint& c);
	void _faceTexturePoints(const Face& face,
		const std::vector<glm::vec2> texture_points,
		const TextureMap& map,
		CanvasPoint& a,
		CanvasPoint& b,
		CanvasPoint& c);

	bool _verifyRaytrace(const glm::vec3& v) const;

	void _drawWire(DrawingWindow& window,
		const Element& elem,
		const std::vector<glm::vec3>& points);
	void _drawRaster(DrawingWindow& window, 
		const Element& elem, 
		const std::vector<glm::vec3>& points, 
		const Material& material);
	void _drawRaytraced(DrawingWindow& window);

	void print(std::string message);

public:

	Scene();
	Scene(glm::vec3 camera_pos, float focal_length);

	void draw(DrawingWindow& window);

	void translate(glm::vec3 v);
	void rotateCamera(glm::vec3 r);
	void rotateWorld(glm::vec3 r);

	void setRenderMode(RenderMode mode);

	void loadObject(std::string name, 
		float load_scale, float draw_scale);

};
