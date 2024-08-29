#include "UniformBuffer.h"

void UniformBuffer::_SetValue(uint32_t offset, uint32_t size, const void* value) {
	glBindBuffer(GL_UNIFORM_BUFFER, buffer_id);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, size, value);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

UniformBuffer::UniformBuffer(uint32_t size) {
	glGenBuffers(1, &buffer_id);

	glBindBuffer(GL_UNIFORM_BUFFER, buffer_id);
	glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

UniformBuffer::~UniformBuffer() {
	glDeleteBuffers(1, &buffer_id);
}

void UniformBuffer::Bind(uint32_t binding_point) {
	glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, buffer_id);
}

void UniformBuffer::SetValue(uint32_t offset, float value) {
	_SetValue(offset, 4, &value);
}

void UniformBuffer::SetValue(uint32_t offset, int value) {
	_SetValue(offset, 4, &value);
}

void UniformBuffer::SetValue(uint32_t offset, bool value) {
	int value_4byte = value;
	_SetValue(offset, 4, &value_4byte);
}

void UniformBuffer::SetValue(uint32_t offset, const vec3& value) {
	_SetValue(offset, sizeof(vec3), &value);
}

void UniformBuffer::SetValue(uint32_t offset, const mat4& value) {
	_SetValue(offset, sizeof(mat4), &value);
}
