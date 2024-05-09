#pragma once

#include "framework.h"
//#include "UniformField.h"
class Shader;

struct Uniform {
	std::string name;
	Shader* shader;
	bool applied;
	union {
		vec3 val_vec3;
		mat4 val_mat4;
		float val_float;
		int val_int;
		bool val_bool;
	};
	void (Shader::*SetUniform)(Uniform&);

	Uniform() {}
};

class Shader {
private:
	uint32_t Id;

	std::string vertexCode;
	std::string fragmentCode;

	static uint32_t LAST_PROGRAM_LOADED;

	void Compile();
	void ReadVertexShader(const char* vertex_path);
	void ReadFragmentShader(const char* fragment_path);
public:
	Shader(const char* vertex_path, const char* fragment_path);
	void Activate();

	void SetBool(Uniform& uniform);
	void SetInt(Uniform& uniform);
	void SetFloat(Uniform& uniform);
	void SetVec3(Uniform& uniform);
	void SetMat4(Uniform& uniform);
	std::string GetCode() {
		std::string concat = vertexCode + fragmentCode;
		return concat;
	}
};