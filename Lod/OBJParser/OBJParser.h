#pragma once
#include "../Mesh.h"
#include <string>

static class OBJParser {
private:
	static void ParseVertices(std::vector<std::vector<std::string>>& input, std::vector<vec3>& output);
	static void ParseFaces(std::vector<std::vector<std::string>>& input, std::vector<Face>& output);
	static void ReadLines(const char* file_name, std::vector<std::string>& output);
	static void Tokenize(std::vector<std::string>& input, std::vector<std::vector<std::string>>& output);
public:
	static Mesh* Parse(const char* file_name);
};