#pragma once
#include <string>
#include "framework.h"
#include <sstream>
#include "Shader.h"
#include <unordered_map>

class UniformField {
private:
	std::vector<Uniform> uniforms;

	void SeparateLines(const std::string& text, std::vector<std::string>& output) {
		output.clear();
		std::stringstream stream(text);
		std::string line;
		while (std::getline(stream, line)) {
			output.push_back(line);
		}
	}
	void TokenizeAndFilter(std::vector<std::string>& input, std::vector<std::vector<std::string>>& output) {
		output.clear();
		for (std::string line : input) {
			if (line.empty()) {
				continue;
			}

			std::vector<std::string> tokens;
			std::string token;
			for (int i = 0; i < line.length(); i++) {
				if (line[i] == ' ') {
					if (!token.empty()) {
						tokens.push_back(token);
						token.clear();
					}
				}
				else {
					token.push_back(line[i]);
				}
			}
			if (!token.empty()) {
				tokens.push_back(token);
			}
			if (tokens.size() >= 3 && tokens[0] == "uniform") {
				if (tokens[2].back() == ';') {
					tokens[2].pop_back();
				}
				output.push_back(tokens);
			}
		}
	}

	Uniform* Search(const std::string& name) {
		for (Uniform& uni : uniforms) {
			if (uni.name == name) {
				return &uni;
			}
		}
		return nullptr;
	}
public:
	UniformField() {}

	void AddUniform(std::string& name, std::string& type, Shader* _shader) {
		Uniform new_uni;
		new_uni.name = name;
		new_uni.shader = _shader;
		new_uni.applied = false;

		if (type == "float") {
			new_uni.val_float = 0;
			new_uni.SetUniform = &Shader::SetFloat;
		}
		else if (type == "int") {
			new_uni.val_int = 0;
			new_uni.SetUniform = &Shader::SetInt;
		}
		else if (type == "bool") {
			new_uni.val_bool = false;
			new_uni.SetUniform = &Shader::SetBool;
		}
		else if (type == "vec3") {
			new_uni.val_vec3 = { 0, 0, 0 };
			new_uni.SetUniform = &Shader::SetVec3;
		}
		else if (type == "mat4") {
			new_uni.val_mat4 = IdentityMatrix();
			new_uni.SetUniform = &Shader::SetMat4;
		}
		else return;

		uniforms.push_back(new_uni);
	}

	void RegisterShader(Shader* shader) {
		uniforms.clear();
		std::vector<std::string> lines;
		SeparateLines(shader->GetCode(), lines);

		std::vector<std::vector<std::string>> tokenized;
		TokenizeAndFilter(lines, tokenized);

		for (std::vector<std::string>& tokens : tokenized) {
			if (tokens.size() == 3) {
				AddUniform(tokens[2], tokens[1], shader);
			}
		}
	}

	UniformField(Shader* shader) {
		RegisterShader(shader);
	}

	vec3 GetUniformVec3(const std::string& name) {
		Uniform* uniform = Search(name);
		if (uniform) {
			return uniform->val_vec3;
		}
		return vec3(0, 0, 0);
	}

	bool SetUniform(const std::string& name, int value) {
		Uniform* uniform = Search(name);
		if (uniform) {
			uniform->val_int = value;
			uniform->applied = false;
			return true;
		}
		return false;
	}

	bool SetUniform(const std::string& name, float value) {
		Uniform* uniform = Search(name);
		if (uniform) {
			uniform->val_float = value;
			uniform->applied = false;
			return true;
		}
		return false;
	}

	bool SetUniform(const std::string& name, bool value) {
		Uniform* uniform = Search(name);
		if (uniform) {
			uniform->val_bool = value;
			uniform->applied = false;
			return true;
		}
		return false;
	}

	bool SetUniform(const std::string& name, const vec3& value) {
		Uniform* uniform = Search(name);
		if (uniform) {
			uniform->val_vec3 = value;
			uniform->applied = false;
			return true;
		}
		return false;
	}

	bool SetUniform(const std::string& name, const mat4& value) {
		Uniform* uniform = Search(name);
		if (uniform) {
			uniform->val_mat4 = value;
			uniform->applied = false;
			return true;
		}
		return false;
	}

	void Apply() {
		for (Uniform& uniform : uniforms) {
			if (!uniform.applied) {
				(uniform.shader->*(uniform.SetUniform))(uniform);
				uniform.applied = true;
			}
		}
	}

	std::vector<std::string> GetUniforms() {
		std::vector<std::string> names;
		for (Uniform& uniform : uniforms) {
			names.push_back(uniform.name);
		}
		return names;
	}
};