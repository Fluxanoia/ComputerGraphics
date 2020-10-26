#include "main.hpp"

#include "maths.hpp"
#include "render.hpp"

Main::Main(int width, int height) : width{ width }, height{ height } {
	window = DrawingWindow{ width, height, false };

	cbox.load("cornell-box.obj", 0.4f);
}

void Main::run() {
	SDL_Event event;
	running = true;
	while (running) {
		if (window.pollForInputEvents(event)) {
			this->_handleEvent(event);
		}
		this->_update();
		window.clearPixels();
		this->_draw();
		window.renderFrame();
	}
}

void Main::_draw() {
	cbox.draw(window, { 0, 0, 4 }, 2, 300);
}

void Main::_update() { }

void Main::_handleEvent(SDL_Event e) {
}

int main(int n, char *args[]) {
	Main m{ 512, 512 };
	m.run();
	return 0;
}
