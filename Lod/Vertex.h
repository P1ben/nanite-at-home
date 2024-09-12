#pragma once
#include "framework.h"

struct Vertex {
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;

	Vertex() {
		position = vec3(0, 0, 0);
		normal   = vec3(0, 0, 0);
		color    = vec3(0, 0, 0);
		uv       = vec2(0, 0);
	}

	Vertex(const vec3& _position, const vec3& _normal, const vec3& _color, const vec2& _uv = vec2(0, 0)) {
		position = _position;
		normal   = _normal;
		color    = _color;
		uv       = _uv;
	}

	// Only mathces uv and position
	bool operator == (const Vertex& other) const {
		return position == other.position  && uv == other.uv;
	}
};

template <>
struct std::hash<Vertex>
{
	std::size_t operator()(const Vertex& vertex) const
	{
		std::size_t pos_x = std::hash<float>()(vertex.position.x);
		std::size_t pos_y = std::hash<float>()(vertex.position.y);
		std::size_t pos_z = std::hash<float>()(vertex.position.z);

		std::size_t uv_x = std::hash<float>()(vertex.uv.x);
		std::size_t uv_y = std::hash<float>()(vertex.uv.y);

		return pos_x ^ (pos_y << 1) ^ (pos_z << 2) ^ (uv_x << 3) ^ (uv_y << 4);
	}
};