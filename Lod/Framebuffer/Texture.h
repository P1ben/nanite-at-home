#pragma once
#include <cstdint>
#include <glew.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

class Texture {
private:
	uint32_t tx_id;
	uint32_t size_x;
	uint32_t size_y;

public:
	Texture(uint32_t size_x, uint32_t size_y) {
		this->size_x = size_x;
		this->size_y = size_y;

		// Create texture object
		glGenTextures(1, &tx_id);

		// Create texture
		glBindTexture(GL_TEXTURE_2D, tx_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size_x, size_y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Unbind texture
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	uint32_t GetID() {
		return tx_id;
	}

	void SaveAsJPG(const char* file_name) {
		uint8_t* data = new uint8_t[size_x * size_y * 4 * sizeof(uint32_t)];

		glBindTexture(GL_TEXTURE_2D, tx_id);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		stbi_flip_vertically_on_write(true);
		if (!stbi_write_jpg(file_name, size_x, size_y, 4, data, 100)) {
			std::cout << "ERROR::FRAMEBUFFER:: Could not save texture contents to file!" << std::endl;
		}

		delete[] data;
	}

	~Texture() {
		glDeleteTextures(1, &tx_id);
	}
};