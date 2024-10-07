#include "ComputeShader.h"
#include <iostream>
#include <fstream>
#include <sstream>

ComputeShader::ComputeShader(const char* shader_path) {
	std::string shader_code;
	std::ifstream shader_file;

	shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		shader_file.open(shader_path);
		std::stringstream shader_stream;

		shader_stream << shader_file.rdbuf();
		shader_file.close();

		shader_code = shader_stream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}

	const char* shader_code_cstr = shader_code.c_str();

	//std::cout << shader_code_cstr << std::endl;

	unsigned int compute_shader;
	int success;
	char info_log[512];

	compute_shader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(compute_shader, 1, &shader_code_cstr, NULL);
	glCompileShader(compute_shader);

	glGetShaderiv(compute_shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(compute_shader, 512, NULL, info_log);
		std::cout << "ERROR::SHADER::COMPUTE::COMPILATION_FAILED\n" << info_log << std::endl;
	}

	id = glCreateProgram();
	glAttachShader(id, compute_shader);
	glLinkProgram(id);

	glGetProgramiv(id, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(id, 512, NULL, info_log);
		std::cout << "ERROR::SHADER::COMPUTE::LINKING_FAILED\n" << info_log << std::endl;
	}

	glDeleteShader(compute_shader);
}

ComputeShader::~ComputeShader() {
	glDeleteProgram(id);
}

void ComputeShader::Use() {
	glUseProgram(id);
}

void ComputeShader::Dispatch(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ) {
	glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	//glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//glFlush();
	//glFinish();
}
