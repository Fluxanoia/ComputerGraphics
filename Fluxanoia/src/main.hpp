#pragma once

#include <vector>

#include <Colour.h>
#include <TextureMap.h>
#include <DrawingWindow.h>
#include <CanvasTriangle.h>

#include "maths.hpp"
#include "scene.hpp"

class Main {
private:

	const float tra_fac{ 0.05f };
	const float rot_fac{ static_cast<float>(PI) / 100.0f };

	const int width;
	const int height;

	bool running{ false };

	Scene scene{ { 0, 0, 4 }, 2 };
	DrawingWindow window;

	void _update();
	void _draw();
	void _handleEvent(SDL_Event e);

public:

	Main(int width = 360, int height = 240);
	void run();

};
