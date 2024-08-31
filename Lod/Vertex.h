#pragma once
#include "framework.h"

struct Vertex {
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;

	Vertex(const vec3& _position, const vec3& _normal, const vec3& _color, const vec2& _uv = vec2(0, 0)) {
		position = _position;
		normal   = _normal;
		color    = _color;
		uv       = _uv;
	}
};