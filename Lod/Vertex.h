#pragma once
#include "framework.h"

struct Vertex {
	vec3 position;
	vec3 normal;
	vec3 color;

	Vertex(const vec3& _position, const vec3& _normal, const vec3& _color) {
		position = _position;
		normal = _normal;
		color = _color;
	}
};