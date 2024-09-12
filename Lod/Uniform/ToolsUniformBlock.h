#pragma once
#include "UniformBuffer.h"

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

class ToolsUniformBlock {
private:
	UniformBuffer* unibuffer;

public:
	ToolsUniformBlock();
	~ToolsUniformBlock();
	void SetUVXOffset(const float uvYOffset);
	void SetUVYOffset(const float uvYOffset);
	void Bind();
};