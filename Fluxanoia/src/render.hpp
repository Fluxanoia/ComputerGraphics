#pragma once

#include <Colour.h>
#include <TextureMap.h>
#include <CanvasPoint.h>
#include <DrawingWindow.h>
#include <CanvasTriangle.h>

#include "maths.hpp"

enum class RenderMode { WIRE, RASTER, RAYTRACED };

namespace Render {
	
	bool in(DrawingWindow& window, glm::vec2 coord);
	bool in(DrawingWindow& window, glm::vec3 coord);
	bool in(DrawingWindow& window, CanvasPoint point);
	bool in(DrawingWindow& window, CanvasPoint p1, CanvasPoint p2);
	bool in(DrawingWindow& window, CanvasTriangle tri);

	uint32_t _getTextureColour(TextureMap& map,
		float x, float y);

	std::vector<glm::vec3> _coordifyLine(
		CanvasPoint& p1, CanvasPoint& p2);
	std::vector<glm::vec2> _coordifyLine(size_t steps,
		TexturePoint& p1, TexturePoint& p2);

	std::vector<glm::vec2> _coordifyTriangleTop(
		CanvasPoint top,
		CanvasPoint middle_1,
		CanvasPoint middle_2);
	std::vector<glm::vec2> _coordifyTriangleBottom(
		CanvasPoint middle_1,
		CanvasPoint middle_2,
		CanvasPoint bottom);
	std::vector<glm::vec2> _coordifyTriangleTop(
		size_t steps,
		TexturePoint top,
		TexturePoint middle_1,
		TexturePoint middle_2);
	std::vector<glm::vec2> _coordifyTriangleBottom(
		size_t steps,
		TexturePoint middle_1,
		TexturePoint middle_2,
		TexturePoint bottom);

	void drawLine(DrawingWindow& window,
		CanvasPoint p1, CanvasPoint p2, 
		Colour c, float alpha = 255,
		std::vector<float>* depth = nullptr);

	void mapLine(DrawingWindow& window,
		CanvasPoint p1, CanvasPoint p2,
		TextureMap map,
		std::vector<float>* depth = nullptr);

	void drawTriangle(DrawingWindow& window,
		CanvasTriangle triangle,
		Colour c, float alpha = 255);

	void _pointifyTriangle(
		CanvasTriangle& triangle,
		CanvasPoint& top, 
		CanvasPoint& middle_1,
		CanvasPoint& middle_2,
		CanvasPoint& bottom);

	void fillTriangle(DrawingWindow& window,
		CanvasTriangle triangle,
		Colour c, float alpha = 255,
		std::vector<float>* depth = nullptr);

	void mapTriangle(DrawingWindow& window,
		CanvasTriangle triangle,
		TextureMap map, 
		std::vector<float>* depth = nullptr);

	void renderMap(DrawingWindow& window, 
		TextureMap map);

};
