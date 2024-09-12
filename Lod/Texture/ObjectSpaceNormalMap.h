#pragma once
#include "Texture.h"

static constexpr uint32_t OBJECT_SPACE_NORMAL_MAP_TEXTURE_UNIT = 1;

class ObjectSpaceNormalMap {
private:
	Texture* inner_texture;

public:
	ObjectSpaceNormalMap(const char* file_path, bool flipped_vertically = false) {
		inner_texture = new Texture(file_path, flipped_vertically);
	}

	~ObjectSpaceNormalMap() {
		delete inner_texture;
	}

	void Bind() {
		inner_texture->BindToTextureUnit(OBJECT_SPACE_NORMAL_MAP_TEXTURE_UNIT);
	}
};