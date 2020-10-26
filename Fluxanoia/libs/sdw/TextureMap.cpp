#include "TextureMap.h"

#include <SDL_image.h>

TextureMap::TextureMap() = default;
TextureMap::TextureMap(const std::string & filename) {
	_load(filename);
	//_SDL_load(filename);
}

void TextureMap::_load(const std::string & filename) {
	std::ifstream inputStream(filename, std::ifstream::binary);
	std::string nextLine;
	// Get the "P6" magic number
	std::getline(inputStream, nextLine);
	// Read the width and height line
	std::getline(inputStream, nextLine);
	// Skip over any comment lines !
	while (nextLine.at(0) == '#') std::getline(inputStream, nextLine);
	auto widthAndHeight = split(nextLine, ' ');
	if (widthAndHeight.size() != 2)
		throw std::invalid_argument("Failed to parse width and height line, line was `" + nextLine + "`");

	width = std::stoi(widthAndHeight[0]);
	height = std::stoi(widthAndHeight[1]);
	// Read the max value (which we assume is 255)
	std::getline(inputStream, nextLine);

	pixels.resize(width * height);
	for (size_t i = 0; i < width * height; i++) {
		int red = inputStream.get();
		int green = inputStream.get();
		int blue = inputStream.get();
		pixels[i] = ((255 << 24) + (red << 16) + (green << 8) + (blue));
	}
	inputStream.close();
}
void TextureMap::_SDL_load(const std::string & filename) {
	SDL_Surface* surface{ IMG_Load(filename.c_str()) };
	this->width = surface->w;
	this->height = surface->h;
	pixels.resize(width * height);
	uint8_t* raw{ (uint8_t*) surface->pixels };
	for (size_t index{ 0 }; index < pixels.size(); index++) {
		uint8_t r{ raw[index * 3] };
		uint8_t g{ raw[index * 3 + 1] };
		uint8_t b{ raw[index * 3 + 2] };
		this->pixels.at(index) = (255 << 24) + (r << 16) + (g << 8) + b;
	}
	SDL_FreeSurface(surface);
}

std::ostream& operator<<(std::ostream & os, const TextureMap & map) {
	os << "(" << map.width << " x " << map.height << ")";
	return os;
}
