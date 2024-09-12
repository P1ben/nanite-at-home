#pragma once
#include "Shader.h"

class ShaderMaterial {
private:
	Shader* shader;

public:
	ShaderMaterial() {
		shader = nullptr;
	}

	void SetShader(Shader* shader) {
		if (shader) {
			this->shader = shader;
		}
	}

	Shader* GetShader() {
		return shader;
	}

	ShaderMaterial(Shader* shader) {
		SetShader(shader);
	}

	void Activate() {
		if (shader) {
			shader->Activate();
		}
	}
};