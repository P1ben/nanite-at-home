#pragma once
#include "UniformBuffer.h"

/*
* ------------------------------------------
| Reference:
* ------------------------------------------
layout (std140, binding = 2) uniform Object {
	mat4 modelMatrix;
	mat4 modelMatrixInverse;
	vec3 drawColor;
	bool useTrueColor;

	bool useColorTexture;
	bool useObjectSpaceNormalTexture;
};
* ------------------------------------------
*/

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
	void SetUseColorTexture(bool useColorTexture);
	void SetUseObjectSpaceNormalTexture(bool useObjectSpaceNormalTexture);
};