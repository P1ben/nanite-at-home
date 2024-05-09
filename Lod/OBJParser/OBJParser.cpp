#include <fstream>
#include "OBJParser.h"


void OBJParser::ReadLines(const char* file_name, std::vector<std::string>& output) {
	output.clear();
	std::ifstream file(file_name);
	std::string s;
	while (getline(file, s)) {
		if (!s.empty()) {
			output.push_back(s);
		}
	}
}

void OBJParser::Tokenize(std::vector<std::string>& input, std::vector<std::vector<std::string>>& output) {
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

		output.push_back(tokens);
	}
}

void OBJParser::ParseVertices(std::vector<std::vector<std::string>>& input, std::vector<vec3>& output) {
	output.clear();
	for (std::vector<std::string> line : input) {
		if (line.size() >= 4) {
			if (line[0] == "v") {
				output.push_back({(float)atof(line[1].c_str()), (float)atof(line[2].c_str()), (float)atof(line[3].c_str())});
			}
		}
	}
}

void OBJParser::ParseFaces(std::vector<std::vector<std::string>>& input, std::vector<Face>& output) {
	output.clear();
	for (std::vector<std::string> line : input) {
		if (line.size() == 4) {
			if (line[0] == "f") {
				output.push_back(Face(atoi(line[1].c_str()) - 1, atoi(line[2].c_str()) - 1, atoi(line[3].c_str()) - 1));
			}
		}
	}
}

//Mesh* OBJParser::Parse(const char* file_name) {
//	std::vector<std::string> lines;
//	ReadLines(file_name, lines);
//
//	std::vector<std::vector<std::string>> token_lists;
//	Tokenize(lines, token_lists);
//
//	std::vector<vec3> vertices;
//	ParseVertices(token_lists, vertices);
//
//	std::vector<Face> faces;
//	ParseFaces(token_lists, faces);
//
//	return new Mesh(vertices, faces);
//}