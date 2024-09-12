#pragma once
#include "Texture.h"

static constexpr uint32_t COLOR_MAP_TEXTURE_UNIT = 0;

class ColorMap {
private:
	Texture* inner_texture;

public:
	ColorMap(const char* file_path, bool flipped_vertically = false) {
		inner_texture = new Texture(file_path, flipped_vertically);
	}

	~ColorMap() {
		delete inner_texture;
	}

	void Bind() {
		inner_texture->BindToTextureUnit(COLOR_MAP_TEXTURE_UNIT);
	}
};