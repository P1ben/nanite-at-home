#include "CameraUniformBlock.h"

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

enum : uint32_t {
    VIEW_MATRIX_OFFSET          = 0,
    PROJ_MATRIX_OFFSET          = VIEW_MATRIX_OFFSET + sizeof(mat4),
    CAMERA_POSITION_OFFSET      = PROJ_MATRIX_OFFSET + sizeof(mat4),
};

static constexpr uint32_t CAMERA_BINDING_POINT = 1;

CameraUniformBlock::CameraUniformBlock() {
    uint32_t mat4_size = sizeof(mat4);
    uint32_t vec3_size = sizeof(vec4);

    uint32_t full_buffer_size = 2 * mat4_size + vec3_size;

    unibuffer = new UniformBuffer(full_buffer_size);
}

CameraUniformBlock::~CameraUniformBlock() {
    delete unibuffer;
}

void CameraUniformBlock::SetViewMatrix(const mat4& viewMatrix) {
    unibuffer->SetValue(VIEW_MATRIX_OFFSET, viewMatrix);
}

void CameraUniformBlock::SetProjMatrix(const mat4& projMatrix) {
    unibuffer->SetValue(PROJ_MATRIX_OFFSET, projMatrix);
}

void CameraUniformBlock::SetCameraPosition(const vec3& cameraPosition) {
    unibuffer->SetValue(CAMERA_POSITION_OFFSET, cameraPosition);
}

void CameraUniformBlock::Bind() {
    unibuffer->Bind(CAMERA_BINDING_POINT);
}
