#pragma once
#include <glew.h>
#include <cstdint>

class ComputeShader {
private:
	uint32_t id;

public:
	ComputeShader(const char* shader_path);
	~ComputeShader();

	void Use();
	void Dispatch(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ);
};