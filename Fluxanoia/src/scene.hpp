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

	glm::vec3 camera_pos{ 0.0f, 0.0f, 0.0f };
	glm::mat3 calibration{ 
		glm::vec3{ 1.0f, 0.0f, 0.0f },
		glm::vec3{ 0.0f, 1.0f, 0.0f },
		glm::vec3{ 0.0f, 0.0f, 1.0f },
	};
	glm::mat3 extrinsic{
		glm::vec3{ 1.0f, 0.0f, 0.0f },
		glm::vec3{ 0.0f, -1.0f, 0.0f },
		glm::vec3{ 0.0f, 0.0f, 1.0f }
	};
	std::vector<float> depth{ };

	float specular_power{ 16.0f };
	float specular_cull{ 0.8f };
	float light_strength{ 3.0f };
	const float ambient_light{ 0.05f };
	glm::vec3 light{ -0.2f, 0.8f, 0.5f };
	//glm::vec3 light{ 0, 1.2f, 0 };
	bool _occluded(const glm::vec3 v, 
		const Element& celem, const Face& cface);
	float _brightnessPhong(const glm::vec3 v,
		const Element& celem, const Face& cface,
		const glm::vec3 solution);
	float _brightnessGouraud(const glm::vec3 v, 
		const Element& celem, const Face& cface,
		const glm::vec3 solution);
	float _brightness(const glm::vec3 v,
		const Element& celem, const Face& cface,
		const glm::vec3 normal);
	void _darken(Colour& c, float f);

	std::vector<std::pair<std::string, TextureMap>> textures{ };
	std::vector<std::pair<Object, float>> objects{ };
	std::vector<Material> materials{ };
	void _loadMaterials(const std::string filename);

	glm::mat4 _getExtrinsicMatrix() const;
	glm::vec3 _transformPoint(DrawingWindow& w, 
		glm::vec3 p, float scale);
	void _transformPoints(DrawingWindow& w,
		float scale,
		std::vector<glm::vec3> p,
		std::vector<glm::vec3>& out);

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

	bool _raytrace(const glm::vec3& from,
		const glm::vec3& ray,
		const glm::vec3& a,
		const glm::vec3& b,
		const glm::vec3& c,
		glm::vec3* sr = nullptr,
		glm::vec3* ur = nullptr,
		glm::vec3* vr = nullptr) const;

	void _drawWire(DrawingWindow& window,
		const Element& elem,
		const std::vector<glm::vec3>& points);
	void _drawRaster(DrawingWindow& window, 
		const Element& elem, 
		const std::vector<glm::vec3>& points, 
		const Material& material);
	void _drawRaytraced(DrawingWindow& window);

public:

	Scene();
	Scene(glm::vec3 camera_pos, float focal_length);

	void draw(DrawingWindow& window);

	void translate(glm::vec3 v);
	void rotateCamera(glm::vec3 r);
	void rotateWorld(glm::vec3 r);
	void lookAt(glm::vec3 l);

	void setRenderMode(RenderMode mode);

	void loadObject(std::string name, 
		float load_scale, float draw_scale);

};
