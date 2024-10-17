#include "ToolsUniformBlock.h"

/*
* ------------------------------------------
| Reference:
* ------------------------------------------
layout (std140, binding = 3) uniform Tools {
    float uvXOffset;
    float uvYOffset;
};
* ------------------------------------------
*/

enum : uint32_t {
    UV_X_OFFSET_OFFSET = 0,
    UV_Y_OFFSET_OFFSET = UV_X_OFFSET_OFFSET + 4,
};

static constexpr uint32_t TOOLS_BINDING_POINT = 4;

ToolsUniformBlock::ToolsUniformBlock() {
    uint32_t float_size = 4;

    uint32_t full_buffer_size = 2 * float_size;

    unibuffer = new UniformBuffer(full_buffer_size);
}

ToolsUniformBlock::~ToolsUniformBlock() {
    delete unibuffer;
}

void ToolsUniformBlock::SetUVXOffset(const float uvXOffset) {
    unibuffer->SetValue(UV_X_OFFSET_OFFSET, uvXOffset);
}

void ToolsUniformBlock::SetUVYOffset(const float uvYOffset) {
    unibuffer->SetValue(UV_Y_OFFSET_OFFSET, uvYOffset);
}

void ToolsUniformBlock::Bind() {
    unibuffer->Bind(TOOLS_BINDING_POINT);
}
