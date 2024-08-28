#pragma once
#include <cstdint>
#include <glew.h>
#include "../framework.h"

class UniformBuffer {
private:
	uint32_t buffer_id;

	void _SetValue(uint32_t offset, uint32_t size, const void* value);

public:
	UniformBuffer(uint32_t size);;
	~UniformBuffer();

	void Bind(uint32_t binding_point);

	void SetValue(uint32_t offset, float value);
	void SetValue(uint32_t offset, int value);
	void SetValue(uint32_t offset, bool value);
	void SetValue(uint32_t offset, const vec3& value);
	void SetValue(uint32_t offset, const mat4& value);
};