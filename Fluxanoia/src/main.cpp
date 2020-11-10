#include "main.hpp"

#include "maths.hpp"
#include "render.hpp"

Main::Main(int width, int height) : width{ width }, height{ height } {
	window = DrawingWindow{ width, height, false };
	scene.loadObject("textured-cornell-box.obj", 0.4f, 100);
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
	scene.draw(window);
	// scene.rotateWorld({ 0, rot_fac / 2, 0 });
}
void Main::_update() { }
void Main::_handleEvent(SDL_Event e) { 
	switch (e.type) {
	case SDL_KEYDOWN:
		glm::vec3 v{ 0, 0, 0 };
		glm::vec3 r{ 0, 0, 0 };
		glm::vec3 o{ 0, 0, 0 };
		switch (e.key.keysym.sym) {
		case SDLK_d:      v[0] -= tra_fac; break;
		case SDLK_a:      v[0] += tra_fac; break;
		case SDLK_LSHIFT: v[1] -= tra_fac; break;
		case SDLK_SPACE:  v[1] += tra_fac; break;
		case SDLK_w:      v[2] -= tra_fac; break;
		case SDLK_s:      v[2] += tra_fac; break;

		case SDLK_DOWN:         r[0] -= rot_fac; break;
		case SDLK_UP:           r[0] += rot_fac; break;
		case SDLK_LEFT:         r[1] -= rot_fac; break;
		case SDLK_RIGHT:        r[1] += rot_fac; break;
		case SDLK_RIGHTBRACKET: r[2] -= rot_fac; break;
		case SDLK_LEFTBRACKET:  r[2] += rot_fac; break;

		case SDLK_i: o[0] -= rot_fac; break;
		case SDLK_k: o[0] += rot_fac; break;
		case SDLK_l: o[1] -= rot_fac; break;
		case SDLK_j: o[1] += rot_fac; break;
		case SDLK_u: o[2] -= rot_fac; break;
		case SDLK_o: o[2] += rot_fac; break;

		case SDLK_z: scene.setRenderMode(RenderMode::WIRE); break;
		case SDLK_x: scene.setRenderMode(RenderMode::RASTER); break;
		case SDLK_c: scene.setRenderMode(RenderMode::RAYTRACED); break;

		case SDLK_r: scene.lookAt({ 0, 0, 0 });
		}
		if (glm::length(v) != 0) scene.translate(v);
		if (glm::length(o) != 0) scene.rotateCamera(o);
		if (glm::length(r) != 0) {
			scene.rotateWorld(r);
			scene.lookAt({ 0, 0, 0 });
		}
		break;
	}
}

int main(int n, char *args[]) {
	Main m{ 512, 512 };
	m.run();
	return 0;
}
