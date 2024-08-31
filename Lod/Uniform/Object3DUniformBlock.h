#pragma once
#include "UniformBuffer.h"

class Object3DUniformBlock {
private:
	UniformBuffer* unibuffer;

public:
	Object3DUniformBlock();
	~Object3DUniformBlock();

	void Bind();

	void SetModelMatrix(const mat4& modelMatrix);
	void SetModelMatrixInverse(const mat4& modelMatrixInverse);
	void SetDrawColor(const vec3& drawColor);
	void SetUseTrueColor(bool useTrueColor);
};