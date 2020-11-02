#include "render.hpp"

#include <algorithm>

bool Render::in(DrawingWindow& window, glm::vec2 coord) {
	if (!Maths::in(coord[0], 0,
		static_cast<float>(window.width))) return false;
	if (!Maths::in(coord[1], 0,
		static_cast<float>(window.height))) return false;
	return true;
}
bool Render::in(DrawingWindow& window, glm::vec3 coord) {
	return Render::in(window, glm::vec2{ coord[0], coord[1] });
}
bool Render::in(DrawingWindow& window, CanvasPoint point) {
	return Render::in(window, glm::vec2{ point.x, point.y });
}
bool Render::in(DrawingWindow& window, CanvasPoint p1, CanvasPoint p2) {
	return Render::in(window, p1)
		|| Render::in(window, p2);
}
bool Render::in(DrawingWindow& window, CanvasTriangle tri) {
	return Render::in(window, tri.v0())
		|| Render::in(window, tri.v1())
		|| Render::in(window, tri.v2());
}

uint32_t Render::_getTextureColour(TextureMap& map,
	float x, float y) {
	int tx{ static_cast<int>(x) };
	int ty{ static_cast<int>(y) };
	return map.pixels.at(tx + ty * map.width);
}

std::vector<glm::vec3> Render::_coordifyLine(
	CanvasPoint& p1, CanvasPoint& p2) {
	size_t steps{ static_cast<size_t>(
		std::max(abs(p2.x - p1.x), abs(p2.y - p1.y))) };
	return Maths::interpolate3D(
		{ p1.x, p1.y, p1.depth }, { p2.x, p2.y, p2.depth }, steps + 2);
}
std::vector<glm::vec2> Render::_coordifyLine(size_t steps,
	TexturePoint& p1, TexturePoint& p2) {
	return Maths::interpolate2D(
		{ p1.x, p1.y }, { p2.x, p2.y }, steps + 2);
}

std::vector<glm::vec2> Render::_coordifyTriangleTop(
	CanvasPoint top,
	CanvasPoint middle_1,
	CanvasPoint middle_2) {
	return Maths::interpolate2D({ top.x, top.x },
		{ middle_1.x, middle_2.x }, 
		static_cast<size_t>(middle_1.y) 
		- static_cast<size_t>(top.y) + 1);
}
std::vector<glm::vec2> Render::_coordifyTriangleBottom(
	CanvasPoint middle_1,
	CanvasPoint middle_2,
	CanvasPoint bottom) {
	return Maths::interpolate2D({ middle_1.x, middle_2.x },
		{ bottom.x, bottom.x }, 
		static_cast<size_t>(bottom.y) 
		- static_cast<size_t>(middle_1.y) + 1);
}
std::vector<glm::vec2> Render::_coordifyTriangleTop(
	size_t steps,
	TexturePoint top,
	TexturePoint middle_1,
	TexturePoint middle_2) {
	return Maths::interpolate2D({ top.x, top.x },
		{ middle_1.x, middle_2.x }, steps);
}
std::vector<glm::vec2> Render::_coordifyTriangleBottom(
	size_t steps,
	TexturePoint middle_1,
	TexturePoint middle_2,
	TexturePoint bottom) {
	return Maths::interpolate2D({ middle_1.x, middle_2.x },
		{ bottom.x, bottom.x }, steps);
}

void Render::drawLine(DrawingWindow& window,
	CanvasPoint p1, CanvasPoint p2, 
	Colour c, float alpha,
	std::vector<float>* depth) {
	if (!Render::in(window, p1, p2)) return;

	for (auto coord : _coordifyLine(p1, p2)) {
		if (!Render::in(window, coord)) continue;
		const size_t x{ static_cast<size_t>(coord[0]) };
		const size_t y{ static_cast<size_t>(coord[1]) };
		const size_t index{ x + window.width * y };
		if (depth != nullptr) {
			if (depth->at(index) > coord[2]) continue;
			depth->at(index) = coord[2];
		}
		window.setPixelColour(x, y, Maths::pack(c, alpha));
	}
}

void Render::mapLine(DrawingWindow& window,
	CanvasPoint p1, CanvasPoint p2,
	TextureMap map, std::vector<float>* depth) {
	auto coords{ _coordifyLine(p1, p2) };
	auto texture_coords{ _coordifyLine(coords.size(), 
		p1.texturePoint, p2.texturePoint) };
	for (size_t index{ 0 }; index < coords.size(); index++) {
		auto coord{ coords.at(index) };
		auto texture_coord{ texture_coords.at(index) };

		if (!Render::in(window, coord)) continue;
		
		const size_t x{ static_cast<size_t>(coord[0]) };
		const size_t y{ static_cast<size_t>(coord[1]) };
		const size_t depth_index{ x + window.width * y };
		if (depth != nullptr) {
			if (depth->at(depth_index) > coord[2]) continue;
			depth->at(depth_index) = coord[2];
		}

		window.setPixelColour(x, y,
			Render::_getTextureColour(map, 
				texture_coord[0], texture_coord[1]));
	}
}

void Render::drawTriangle(DrawingWindow& window,
	CanvasTriangle triangle, Colour c, float alpha) {
	if (!Render::in(window, triangle)) return;

	Render::drawLine(window,
		triangle.v0(), triangle.v1(), c, alpha);
	Render::drawLine(window,
		triangle.v1(), triangle.v2(), c, alpha);
	Render::drawLine(window,
		triangle.v2(), triangle.v0(), c, alpha);
}

void Render::_pointifyTriangle(CanvasTriangle& t,
	CanvasPoint& top,
	CanvasPoint& middle_1,
	CanvasPoint& middle_2,
	CanvasPoint& bottom) {
	std::vector<CanvasPoint> points{ t.v0(), t.v1(), t.v2() };
	std::sort(points.begin(), points.end(),
		[](CanvasPoint p1, CanvasPoint p2) {
			return p1.y < p2.y;
		}
	);
	top = points.front();
	middle_1 = points.at(1);
	bottom = points.back();

	float ratio{ ((middle_1.y - top.y) / (bottom.y - top.y)) };
	middle_2 = {
		top.x + ratio * (bottom.x - top.x),
		middle_1.y,
		top.depth + ratio * (bottom.depth - top.depth)
	};
	middle_2.texturePoint = {
		top.texturePoint.x + ratio * (bottom.texturePoint.x - top.texturePoint.x),
		top.texturePoint.y + ratio * (bottom.texturePoint.y - top.texturePoint.y)
	};
}

void Render::fillTriangle(DrawingWindow& window,
	CanvasTriangle t, Colour c, float alpha,
	std::vector<float>* depth) {
	
	if (!Render::in(window, t)) return;

	CanvasPoint top, middle_1, middle_2, bottom;
	Render::_pointifyTriangle(t, top, middle_1, middle_2, bottom);

	std::vector<glm::vec2> coords{ 
		Render::_coordifyTriangleTop(top, middle_1, middle_2) };
	std::vector<glm::vec2> depth_coords{
		Maths::interpolate2D({ top.depth, top.depth },
		{ middle_1.depth, middle_2.depth }, coords.size()) };
	for (size_t index{ 0 }; index < coords.size(); index++) {
		auto coord{ coords.at(index) };
		auto depth_coord{ depth_coords.at(index) };
		float y{ top.y + index };
		Render::drawLine(window, 
			{ coord[0], y, depth_coord[0] }, 
			{ coord[1], y, depth_coord[1] },
			c, alpha, depth);
	}

	coords = Render::_coordifyTriangleBottom(middle_1, middle_2, bottom);
	depth_coords = Maths::interpolate2D(
		{ middle_1.depth, middle_2.depth },
		{ bottom.depth, bottom.depth }, coords.size());
	for (size_t index{ 0 }; index < coords.size(); index++) {
		auto coord{ coords.at(index) };
		auto depth_coord{ depth_coords.at(index) };
		float y{ middle_1.y + index };
		Render::drawLine(window,
			{ coord[0], y, depth_coord[0] },
			{ coord[1], y, depth_coord[1] },
			c, alpha, depth);
	}
}

void Render::mapTriangle(DrawingWindow& window,
	CanvasTriangle t, TextureMap map,
	std::vector<float>* depth) {

	if (!Render::in(window, t)) return;

	CanvasPoint top, middle_1, middle_2, bottom;
	Render::_pointifyTriangle(t, top, middle_1, middle_2, bottom);

	std::vector<glm::vec2> coords{
		Render::_coordifyTriangleTop(top, middle_1, middle_2) };
	std::vector<glm::vec2> texture_coords{
		Render::_coordifyTriangleTop(coords.size(), top.texturePoint, 
			middle_1.texturePoint, middle_2.texturePoint) };
	std::vector<glm::vec2> depth_coords{
		Maths::interpolate2D({ top.depth, top.depth },
		{ middle_1.depth, middle_2.depth }, coords.size()) };
	float texture_step_1{ (middle_1.texturePoint.y - top.texturePoint.y)
		/ coords.size() };
	float texture_step_2{ (middle_2.texturePoint.y - top.texturePoint.y)
		/ coords.size() };
	for (size_t index{ 0 }; index < coords.size(); index++) {
		auto coord{ coords.at(index) };
		auto texture_coord{ texture_coords.at(index) };
		auto depth_coord{ depth_coords.at(index) };
		float y{ top.y + index };
		float texture_y_1{ top.texturePoint.y + index * texture_step_1 };
		float texture_y_2{ top.texturePoint.y + index * texture_step_2 };
		CanvasPoint p1{ coord[0], y, depth_coord[0] },
			p2{ coord[1], y, depth_coord[1] };
		p1.texturePoint = { texture_coord[0], texture_y_1 };
		p2.texturePoint = { texture_coord[1], texture_y_2 };
		Render::mapLine(window, p1, p2, map, depth);
	}

	coords = Render::_coordifyTriangleBottom(middle_1, middle_2, bottom);
	texture_coords = Render::_coordifyTriangleBottom(coords.size(),
		middle_1.texturePoint, middle_2.texturePoint, bottom.texturePoint);
	texture_step_1 = (bottom.texturePoint.y - middle_1.texturePoint.y)
		/ coords.size();
	texture_step_2 = (bottom.texturePoint.y - middle_2.texturePoint.y)
		/ coords.size();
	depth_coords = Maths::interpolate2D(
		{ middle_1.depth, middle_2.depth },
		{ bottom.depth, bottom.depth }, coords.size());
	for (size_t index{ 0 }; index < coords.size(); index++) {
		auto coord{ coords.at(index) };
		auto texture_coord{ texture_coords.at(index) };
		auto depth_coord{ depth_coords.at(index) };
		float y{ middle_1.y + index };
		float texture_y_1{ middle_1.texturePoint.y + index * texture_step_1 };
		float texture_y_2{ middle_2.texturePoint.y + index * texture_step_2 };
		CanvasPoint p1{ coord[0], y, depth_coord[0] }, 
			p2{ coord[1], y, depth_coord[1] };
		p1.texturePoint = { texture_coord[0], texture_y_1 };
		p2.texturePoint = { texture_coord[1], texture_y_2 };
		Render::mapLine(window, p1, p2, map, depth);
	}
}

void Render::renderMap(DrawingWindow& window, 
	TextureMap map) {
	for (size_t index{ 0 }; index < map.pixels.size(); index++) {
		auto x{ index % map.width };
		auto y{ (index - x) / map.width };
		if (!Render::in(window, glm::vec2{ x,  y })) continue;
		window.setPixelColour(x, y, map.pixels.at(index));
	}
}
