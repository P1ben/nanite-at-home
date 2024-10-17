#pragma once
#include <cstdint>
#include <glew.h>
#include "../framework.h"
#include "../Face.h"

class SSBO {
private:
	uint32_t id;

public:
	SSBO(uint32_t size) {
		glGenBuffers(1, &id);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	SSBO(uint32_t size, void* data) {
		glGenBuffers(1, &id);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void Resize(uint32_t size) {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, NULL, GL_STREAM_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
	 
	void Bind(uint32_t binding_point) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point, id);
	}

	void Bind() {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
	}

	void BindAsIndexBuffer() {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
	}

	void BindAsVertexBuffer() {
		glBindBuffer(GL_ARRAY_BUFFER, id);
	}

	void ManualIndexFill(const std::vector<Face>& faces) {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
		glBufferData(GL_SHADER_STORAGE_BUFFER, faces.size() * sizeof(Face), faces.data(), GL_STATIC_DRAW);
	}

	void ManualVertexFill(const std::vector<Vertex>& vertices) {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
		glBufferData(GL_SHADER_STORAGE_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
	}

	void ReadData(uint32_t offset, uint32_t size, void* output) {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, output);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void WriteData(uint32_t offset, uint32_t size, void* input) {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, input);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	uint32_t GetId() {
		return id;
	}

	~SSBO() {
		glDeleteBuffers(1, &id);
	}
};