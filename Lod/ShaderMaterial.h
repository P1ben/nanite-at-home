#pragma once
#include "Shader.h"
#include "UniformField.h"

class ShaderMaterial {
private:
	Shader* shader;
	UniformField uniform_field;

public:
	ShaderMaterial() {
		shader = nullptr;
		uniform_field = UniformField();
	}


	void SetShader(Shader* shader) {
		if (shader) {
			this->shader = shader;
			uniform_field.RegisterShader(shader);
		}
	}

	ShaderMaterial(Shader* shader) {
		SetShader(shader);
	}

	template<typename T>
	bool SetUniform(const char* name, const T& value) {
		std::string name_str = name;
		return uniform_field.SetUniform(name_str, value);
	}

	void Activate() {
		if (shader) {
			shader->Activate();
			uniform_field.Apply();
		}
	}

	void ApplyUniforms() {
		if (shader) {
			uniform_field.Apply();
		}
	}

	std::vector<std::string> GetUniforms() {
		return uniform_field.GetUniforms();
	}

	UniformField* GetUniformField() {
		return &uniform_field;
	}
};