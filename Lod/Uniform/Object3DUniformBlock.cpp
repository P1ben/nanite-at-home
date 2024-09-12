#include "Object3DUniformBlock.h"

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

enum : uint32_t {
	MODEL_MATRIX_OFFSET         = 0,
	MODEL_MATRIX_INVERSE_OFFSET = MODEL_MATRIX_OFFSET         + sizeof(mat4),
	DRAW_COLOR_OFFSET           = MODEL_MATRIX_INVERSE_OFFSET + sizeof(mat4),
	USE_TRUE_COLOR_OFFSET       = DRAW_COLOR_OFFSET           + sizeof(vec3),
	USE_COLOR_TEXTURE_OFFSET    = USE_TRUE_COLOR_OFFSET       + 4,
	USE_OBJECT_SPACE_NORMAL_TEXTURE_OFFSET = USE_COLOR_TEXTURE_OFFSET + 4,
};

static constexpr uint32_t OBJECT3D_BINDING_POINT = 2;

Object3DUniformBlock::Object3DUniformBlock() {
	uint32_t mat4_size = sizeof(mat4);
	uint32_t vec3_size = sizeof(vec3);
	uint32_t bool_size = 4;

	uint32_t full_buffer_size = 2 * mat4_size + vec3_size + 3 * bool_size;

	unibuffer = new UniformBuffer(full_buffer_size);
}

Object3DUniformBlock::~Object3DUniformBlock() {
	delete unibuffer;
}

void Object3DUniformBlock::Bind() {
	unibuffer->Bind(OBJECT3D_BINDING_POINT);
}

void Object3DUniformBlock::SetModelMatrix(const mat4& modelMatrix) {
	unibuffer->SetValue(MODEL_MATRIX_OFFSET, modelMatrix);
}

void Object3DUniformBlock::SetModelMatrixInverse(const mat4& modelMatrixInverse) {
	unibuffer->SetValue(MODEL_MATRIX_INVERSE_OFFSET, modelMatrixInverse);
}

void Object3DUniformBlock::SetDrawColor(const vec3& drawColor) {
	unibuffer->SetValue(DRAW_COLOR_OFFSET, drawColor);
}

void Object3DUniformBlock::SetUseTrueColor(bool useTrueColor) {
	unibuffer->SetValue(USE_TRUE_COLOR_OFFSET, useTrueColor);
}

void Object3DUniformBlock::SetUseColorTexture(bool useColorTexture){
	unibuffer->SetValue(USE_COLOR_TEXTURE_OFFSET, useColorTexture);
}

void Object3DUniformBlock::SetUseObjectSpaceNormalTexture(bool useObjectSpaceNormalTexture){
	unibuffer->SetValue(USE_OBJECT_SPACE_NORMAL_TEXTURE_OFFSET, useObjectSpaceNormalTexture);
	printf("Normal offset: %u\n", USE_OBJECT_SPACE_NORMAL_TEXTURE_OFFSET);
}
