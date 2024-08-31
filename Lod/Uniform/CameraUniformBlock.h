#pragma once
#include "UniformBuffer.h"

/*
* ------------------------------------------
| Reference:
* ------------------------------------------
layout (std140, binding = 1) uniform Camera {
    mat4 viewMatrix;
    mat4 projMatrix;
    vec3 cameraPosition;
};
* ------------------------------------------
*/

class CameraUniformBlock {
private:
	UniformBuffer* unibuffer;

public:
	CameraUniformBlock();
    ~CameraUniformBlock();
	void SetViewMatrix(const mat4& viewMatrix);
	void SetProjMatrix(const mat4& projMatrix);
	void SetCameraPosition(const vec3& cameraPosition);
	void Bind();
};