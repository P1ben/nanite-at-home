#pragma once
#include <fstream>
#include <unordered_map>
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>

#include "../StaticMesh.h"
#include "../framework.h"

typedef std::vector<std::string> TokenList;

class ObjReader {
private:
	enum ObjLineType : uint8_t {
		VERTEX,
		FACE,
		TEXTURE,
		NORMAL,
		CLUSTER_SEPARATOR,
		UNKNOWN,
	};

	enum FaceType : uint8_t {
		VERTEX_ONLY,
		VERTEX_TEXTURE,
		VERTEX_NORMAL,
		VERTEX_TEXTURE_NORMAL,
		UNKNOWN_FACE_TYPE,
	};

	struct ObjFace {
		FaceType type;
		int v1, v2, v3;
		int vt1, vt2, vt3;
		int vn1, vn2, vn3;
	};

	// Tokenizes a line
	static void TokenizeLine(const std::string& in_line, TokenList& out_tokens, char separator) {
		out_tokens.clear();
		std::string token;
		for (int i = 0; i < in_line.length(); i++) {
			if (in_line[i] == separator) {
				if (!token.empty()) {
					out_tokens.push_back(token);
					token.clear();
				}
			}
			else {
				token.push_back(in_line[i]);
			}
		}
		if (!token.empty()) {
			out_tokens.push_back(token);
		}
	}

	// Returns the type of the line
	static ObjLineType GetLineType(const TokenList& line_tokens) {
		if (line_tokens.size() == 0) {
			return ObjLineType::UNKNOWN;
		}

		if (line_tokens[0] == "v") {
			return ObjLineType::VERTEX;
		}
		else if (line_tokens[0] == "f") {
			return ObjLineType::FACE;
		}
		else if (line_tokens[0] == "vt") {
			return ObjLineType::TEXTURE;
		}
		else if (line_tokens[0] == "vn") {
			return ObjLineType::NORMAL;
		}
		else if (line_tokens[0] == "new_cluster") {
			return ObjLineType::CLUSTER_SEPARATOR;
		}
		else {
			return ObjLineType::UNKNOWN;
		}
	}

	static vec3 ParseVertex(const TokenList& tokens) {
		if (tokens.size() < 4) {
			std::cout << "ERROR:: ObjReader::ParseVertex: Not enough tokens!" << std::endl;
			return vec3(0.0f);
		}

		return vec3((float)atof(tokens[1].c_str()), (float)atof(tokens[2].c_str()), (float)atof(tokens[3].c_str()));
	}

	static vec3 ParseNormal(const TokenList& tokens) {
		if (tokens.size() < 4) {
			std::cout << "ERROR:: ObjReader::ParseNormal: Not enough tokens!" << std::endl;
			return vec3(0.0f);
		}

		return vec3((float)atof(tokens[1].c_str()), (float)atof(tokens[2].c_str()), (float)atof(tokens[3].c_str()));
	}

	static vec2 ParseTexCoord(const TokenList& tokens) {
		if (tokens.size() < 3) {
			std::cout << "ERROR:: ObjReader::ParseTexCoord: Not enough tokens!" << std::endl;
			return vec2(0.0f);
		}

		return vec2((float)atof(tokens[1].c_str()), (float)atof(tokens[2].c_str()));
	}

	static int CountSlash(const std::string& s) {
		int count = 0;

		for (int i = 0; i < s.size(); i++)
			if (s[i] == '/') count++;

		return count;
	}

	static std::vector<ObjFace> ParseFaceList(const TokenList& tokens) {
		std::vector<ObjFace> faces;
		faces.reserve(tokens.size() - 3);
		TokenList face_tokens_temp(4);

		face_tokens_temp[0] = tokens[0];
		face_tokens_temp[1] = tokens[1];
		face_tokens_temp[2] = tokens[2];
		face_tokens_temp[3] = tokens[3];

		faces.push_back(ParseFace(face_tokens_temp, 0, 0, 0));

		face_tokens_temp[0] = tokens[0];
		face_tokens_temp[1] = tokens[1];
		face_tokens_temp[2] = tokens[3];
		face_tokens_temp[3] = tokens[4];

		faces.push_back(ParseFace(face_tokens_temp, 0, 0, 0));

		return faces;
	}

	static ObjFace ParseFace(const TokenList& tokens, int vertex_offset, int normal_offset, int tex_offset) {
		ObjFace face;
		face.type = FaceType::VERTEX_ONLY;

		if (tokens.size() < 4) {
			std::cout << "ERROR:: ObjReader::ParseFace: Not enough tokens!" << std::endl;
			face.type = FaceType::UNKNOWN_FACE_TYPE;
			return face;
		}

		if (tokens[1].find("//") != std::string::npos) {
			face.type = FaceType::VERTEX_NORMAL;
		}
		else {
			int slash_count = CountSlash(tokens[1]);
			if (slash_count == 1) {
				face.type = FaceType::VERTEX_TEXTURE;
			}
			else if (slash_count == 2) {
				face.type = FaceType::VERTEX_TEXTURE_NORMAL;
			}
			else {
				face.type = FaceType::VERTEX_ONLY;
			}
		}

		if (face.type == FaceType::VERTEX_ONLY) {
			face.v1 = atoi(tokens[1].c_str()) + vertex_offset;
			face.v2 = atoi(tokens[2].c_str()) + vertex_offset;
			face.v3 = atoi(tokens[3].c_str()) + vertex_offset;
			return face;
		}
		
		TokenList face_tokens;
		TokenizeLine(tokens[1], face_tokens, '/');

		switch (face.type) {
			case FaceType::VERTEX_TEXTURE:
				face.v1 = atoi(face_tokens[0].c_str()) + vertex_offset;
				face.vt1 = atoi(face_tokens[1].c_str()) + tex_offset;
				break;
			case FaceType::VERTEX_NORMAL:
				face.v1 = atoi(face_tokens[0].c_str()) + vertex_offset;
				face.vn1 = atoi(face_tokens[1].c_str()) + normal_offset;
				break;
			case FaceType::VERTEX_TEXTURE_NORMAL:
				face.v1 = atoi(face_tokens[0].c_str()) + vertex_offset;
				face.vt1 = atoi(face_tokens[1].c_str()) + tex_offset;
				face.vn1 = atoi(face_tokens[2].c_str()) + normal_offset;
				break;
		}

		TokenizeLine(tokens[2], face_tokens, '/');
		switch (face.type) {
			case FaceType::VERTEX_TEXTURE:
				face.v2 = atoi(face_tokens[0].c_str()) + vertex_offset;
				face.vt2 = atoi(face_tokens[1].c_str()) + tex_offset;
				break;
			case FaceType::VERTEX_NORMAL:
				face.v2 = atoi(face_tokens[0].c_str()) + vertex_offset;
				face.vn2 = atoi(face_tokens[1].c_str()) + normal_offset;
				break;
			case FaceType::VERTEX_TEXTURE_NORMAL:
				face.v2 = atoi(face_tokens[0].c_str()) + vertex_offset;
				face.vt2 = atoi(face_tokens[1].c_str()) + tex_offset;
				face.vn2 = atoi(face_tokens[2].c_str()) + normal_offset;
				break;
		}

		TokenizeLine(tokens[3], face_tokens, '/');
		switch (face.type) {
			case FaceType::VERTEX_TEXTURE:
				face.v3 = atoi(face_tokens[0].c_str()) + vertex_offset;
				face.vt3 = atoi(face_tokens[1].c_str()) + tex_offset;
				break;
			case FaceType::VERTEX_NORMAL:
				face.v3 = atoi(face_tokens[0].c_str()) + vertex_offset;
				face.vn3 = atoi(face_tokens[1].c_str()) + normal_offset;
				break;
			case FaceType::VERTEX_TEXTURE_NORMAL:
				face.v3 = atoi(face_tokens[0].c_str()) + vertex_offset;
				face.vt3 = atoi(face_tokens[1].c_str()) + tex_offset;
				face.vn3 = atoi(face_tokens[2].c_str()) + normal_offset;
				break;
		}

		if (tokens.size() > 4) {
			std::cout << "ERROR:: ObjReader::ParseFace: Too many tokens!" << std::endl;
			face.type = FaceType::UNKNOWN_FACE_TYPE;
		}

		return face;
	}

	static std::string MergeTokenList(const TokenList& token_list) {
		std::string s;
		for (const std::string& token : token_list) {
			s += token + " ";
		}
		return s;
	}

	// TODO: Refactor this
	static StaticMesh* CreateMesh(const std::vector<vec3>& vertices, const std::vector<vec3>& normals, const std::vector<vec2>& tex_coords, const std::vector<ObjFace>& faces) {
		std::unordered_map<Vertex, int> VertexMap;
		std::vector<Vertex> Vertices;
		std::vector<Face>   Faces;

		auto insert_into_vertices = [&](Vertex num) -> int {
			// Check if the number is already in the map
			if (VertexMap.find(num) != VertexMap.end()) {
				return VertexMap[num]; // Return the index if found
			}

			// If not found, insert the number into the vector
			int newIndex = Vertices.size();
			Vertices.push_back(num);

			// Store the index of the number in the map
			VertexMap[num] = newIndex;

			return newIndex; // Return the new index
		};

		for (const ObjFace& face : faces) {
			Vertex v1, v2, v3;

			if (face.v1 >= 0) {
				v1.position = vertices[face.v1 - 1];
			}
			else {
				v1.position = vertices[vertices.size() + face.v1];
			}

			if (face.v2 >= 0) {
				v2.position = vertices[face.v2 - 1];
			}
			else {
				v2.position = vertices[vertices.size() + face.v2];
			}

			if (face.v3 >= 0) {
				v3.position = vertices[face.v3 - 1];
			}
			else {
				v3.position = vertices[vertices.size() + face.v3];
			}

			if (face.type == FaceType::VERTEX_TEXTURE || face.type == FaceType::VERTEX_TEXTURE_NORMAL) {
				if (face.vt1 >= 0) {
					v1.uv = tex_coords[face.vt1 - 1];
				}
				else {
					v1.uv = tex_coords[tex_coords.size() + face.vt1];
				}

				if (face.vt2 >= 0) {
					v2.uv = tex_coords[face.vt2 - 1];
				}
				else {
					v2.uv = tex_coords[tex_coords.size() + face.vt2];
				}

				if (face.vt3 >= 0) {
					v3.uv = tex_coords[face.vt3 - 1];
				}
				else {
					v3.uv = tex_coords[tex_coords.size() + face.vt3];
				}
			}

			if (face.type == FaceType::VERTEX_NORMAL || face.type == FaceType::VERTEX_TEXTURE_NORMAL) {
				if (face.vn1 >= 0) {
					v1.normal = normals[face.vn1 - 1];
				}
				else {
					v1.normal = normals[normals.size() + face.vn1];
				}

				if (face.vn2 >= 0) {
					v2.normal = normals[face.vn2 - 1];
				}
				else {
					v2.normal = normals[normals.size() + face.vn2];
				}

				if (face.vn3 >= 0) {
					v3.normal = normals[face.vn3 - 1];
				}
				else {
					v3.normal = normals[normals.size() + face.vn3];
				}
			}

			bool found  = false;
			int  index1 = insert_into_vertices(v1);
			int  index2 = insert_into_vertices(v2);
			int  index3 = insert_into_vertices(v3);

			Faces.push_back(Face(index1, index2, index3));
		}

		return new StaticMesh(Vertices, Faces);
	}

	static void WriteZippedNaniteMesh(const std::string& save_path, const std::vector<Vertex>& Vertices, const std::vector<std::vector<Face>>& Faces) {
		std::ofstream file(save_path);
		if (!file.is_open()) {
			std::cout << "ERROR:: ObjReader::WriteZippedNaniteMesh: Couldn't open file for writing!" << std::endl;
			return;
		}

		const auto& vertices = Vertices;
		const auto& clusters = Faces;

		// Write vertices
		for (const Vertex& vertex : vertices) {
			file << "v " << vertex.position.x << " " << vertex.position.y << " " << vertex.position.z << std::endl;
		}

		// Write normals
		for (const Vertex& vertex : vertices) {
			file << "vn " << vertex.normal.x << " " << vertex.normal.y << " " << vertex.normal.z << std::endl;
		}

		// Write texture coordinates
		for (const Vertex& vertex : vertices) {
			file << "vt " << vertex.uv.x << " " << vertex.uv.y << std::endl;
		}

		// Write faces
		for (const auto& faces : clusters) {
			file << "new_cluster" << std::endl;
			for (const Face& face : faces) {
				file << "f " << face.a + 1 << "/" << face.a + 1 << "/" << face.a + 1 << " "
					<< face.b + 1 << "/" << face.b + 1 << "/" << face.b + 1 << " "
					<< face.c + 1 << "/" << face.c + 1 << "/" << face.c + 1 << std::endl;
			}
		}

		file.close();
	}

	static void CreateZippedNaniteMesh(const std::string& save_path, std::vector<vec3>& vertices, const std::vector<vec3>& normals, const std::vector<vec2>& tex_coords, const std::vector< std::vector<ObjFace>>& faces) {
		std::unordered_map<Vertex, int> VertexMap;
		std::vector<Vertex>             Vertices;
		std::vector<std::vector<Face>>  Faces(faces.size());

		auto insert_into_vertices = [&](Vertex num) -> int {
			// Check if the number is already in the map
			if (VertexMap.find(num) != VertexMap.end()) {
				return VertexMap[num]; // Return the index if found
			}

			// If not found, insert the number into the vector
			int newIndex = Vertices.size();
			Vertices.push_back(num);

			// Store the index of the number in the map
			VertexMap[num] = newIndex;

			return newIndex; // Return the new index
			};
		int cluster_index = 0;
		for (const std::vector<ObjFace>& face_l : faces) {
			for (const ObjFace& face : face_l) {
				Vertex v1, v2, v3;

				if (face.v1 >= 0) {
					v1.position = vertices[face.v1 - 1];
				}
				else {
					v1.position = vertices[vertices.size() + face.v1];
				}

				if (face.v2 >= 0) {
					v2.position = vertices[face.v2 - 1];
				}
				else {
					v2.position = vertices[vertices.size() + face.v2];
				}

				if (face.v3 >= 0) {
					v3.position = vertices[face.v3 - 1];
				}
				else {
					v3.position = vertices[vertices.size() + face.v3];
				}

				if (face.type == FaceType::VERTEX_TEXTURE || face.type == FaceType::VERTEX_TEXTURE_NORMAL) {
					if (face.vt1 >= 0) {
						v1.uv = tex_coords[face.vt1 - 1];
					}
					else {
						v1.uv = tex_coords[tex_coords.size() + face.vt1];
					}

					if (face.vt2 >= 0) {
						v2.uv = tex_coords[face.vt2 - 1];
					}
					else {
						v2.uv = tex_coords[tex_coords.size() + face.vt2];
					}

					if (face.vt3 >= 0) {
						v3.uv = tex_coords[face.vt3 - 1];
					}
					else {
						v3.uv = tex_coords[tex_coords.size() + face.vt3];
					}
				}

				if (face.type == FaceType::VERTEX_NORMAL || face.type == FaceType::VERTEX_TEXTURE_NORMAL) {
					if (face.vn1 >= 0) {
						v1.normal = normals[face.vn1 - 1];
					}
					else {
						v1.normal = normals[normals.size() + face.vn1];
					}

					if (face.vn2 >= 0) {
						v2.normal = normals[face.vn2 - 1];
					}
					else {
						v2.normal = normals[normals.size() + face.vn2];
					}

					if (face.vn3 >= 0) {
						v3.normal = normals[face.vn3 - 1];
					}
					else {
						v3.normal = normals[normals.size() + face.vn3];
					}
				}

				bool found = false;
				int  index1 = insert_into_vertices(v1);
				int  index2 = insert_into_vertices(v2);
				int  index3 = insert_into_vertices(v3);

				Faces[cluster_index].push_back(Face(index1, index2, index3));
			}
			cluster_index++;
		}
		WriteZippedNaniteMesh(save_path, Vertices, Faces);
	}

public:
	static StaticMesh* ReadObj(const char* file_name) {
		std::ifstream file(file_name);
		std::string s;
		uint32_t line_number = 1;

		TokenList line_tokens;
		line_tokens.reserve(5);

		std::vector<vec3>    vertices;
		std::vector<vec3>    normals;
		std::vector<vec2>    tex_coords;
		std::vector<ObjFace> faces;

		while (getline(file, s)) {
			line_tokens.clear();

			// Fill line_tokens with line tokens
			TokenizeLine(s, line_tokens, ' ');
			ObjLineType line_type = GetLineType(line_tokens);

			switch (line_type) {
			case ObjLineType::VERTEX:
				vertices.push_back(ObjReader::ParseVertex(line_tokens));
				break;
			case ObjLineType::FACE:
				if (line_tokens.size() > 4) {
					std::vector<ObjFace> faces_temp = ObjReader::ParseFaceList(line_tokens);
					for (ObjFace& face : faces_temp) {
						if (face.type != FaceType::UNKNOWN_FACE_TYPE) {
							faces.push_back(face);
						}
					}
				}
				else {
					ObjFace face = ObjReader::ParseFace(line_tokens, 0, 0, 0);
					if (face.type != FaceType::UNKNOWN_FACE_TYPE) {
						faces.push_back(face);
					}				
				}
				break;
			case ObjLineType::TEXTURE:
				tex_coords.push_back(ObjReader::ParseTexCoord(line_tokens));
				break;
			case ObjLineType::NORMAL:
				normals.push_back(ObjReader::ParseNormal(line_tokens));
				break;

			// Handle unknown line types
			case ObjLineType::UNKNOWN:
			default:
				std::cout << "WARNING:: ObjReader found a problem on line " << line_number << " in file " << file_name <<std::endl;
			}
			line_number++;
		}
		return CreateMesh(vertices, normals, tex_coords, faces);
	}

	static void ZipNaniteMesh(const std::string& folder_path) {
		std::vector<std::string> cluster_files;
		std::string config_file;

		for (const auto& entry : std::experimental::filesystem::directory_iterator(folder_path)) {
			std::string path = entry.path().string();
			if (str_has_suffix(path, ".conf")) {
				config_file = path;
			}
			else if (str_has_suffix(path, ".obj")) {
				cluster_files.push_back(path);
			}
		}

		std::sort(cluster_files.begin(), cluster_files.end());

		std::vector<vec3>    vertices;
		std::vector<vec3>    normals;
		std::vector<vec2>    tex_coords;
		std::vector<std::vector<ObjFace>> faces(cluster_files.size());

		int cluster_index = 0;
		int offset = vertices.size();
		for (const auto& file_name : cluster_files) {
			std::ifstream file(file_name);
			std::string s;
			uint32_t line_number = 1;

			TokenList line_tokens;
			line_tokens.reserve(5);

			int vertex_offset = vertices.size();
			int normal_offset = normals.size();
			int tex_offset    = tex_coords.size();
			while (getline(file, s)) {
				line_tokens.clear();

				// Fill line_tokens with line tokens
				TokenizeLine(s, line_tokens, ' ');
				ObjLineType line_type = GetLineType(line_tokens);

				switch (line_type) {
				case ObjLineType::VERTEX:
					vertices.push_back(ObjReader::ParseVertex(line_tokens));
					break;
				case ObjLineType::FACE:
					if (line_tokens.size() > 4) {
						std::vector<ObjFace> faces_temp = ObjReader::ParseFaceList(line_tokens);
						for (ObjFace& face : faces_temp) {
							if (face.type != FaceType::UNKNOWN_FACE_TYPE) {
								faces[cluster_index].push_back(face);
							}
						}
					}
					else {
						ObjFace face = ObjReader::ParseFace(line_tokens, vertex_offset, normal_offset, tex_offset);
						if (face.type != FaceType::UNKNOWN_FACE_TYPE) {
							faces[cluster_index].push_back(face);
						}
					}
					break;
				case ObjLineType::TEXTURE:
					tex_coords.push_back(ObjReader::ParseTexCoord(line_tokens));
					break;
				case ObjLineType::NORMAL:
					normals.push_back(ObjReader::ParseNormal(line_tokens));
					break;

					// Handle unknown line types
				case ObjLineType::UNKNOWN:
				default:
					std::cout << "WARNING:: ObjReader found a problem on line " << line_number << " in file " << file_name << std::endl;
				}
				line_number++;
			}
			cluster_index++;
		}

		CreateZippedNaniteMesh(folder_path + "\\object.objx", vertices, normals, tex_coords, faces);
	}

	static void ReduceToTriangles(const char* input_path, const char* output_path) {
		std::ifstream input_file(input_path);
		if (!input_file.is_open()) {
			std::cout << "ERROR:: ObjReader::ReduceToTriangles: Couldn't open input file!" << std::endl;
			return;
		}

		std::ofstream output_file(output_path);
		if (!output_file.is_open()) {
			std::cout << "ERROR:: ObjReader::ReduceToTriangles: Couldn't open output file!" << std::endl;
			return;
		}

		std::string s;
		uint32_t line_number = 1;

		TokenList line_tokens;
		line_tokens.reserve(5);

		while (getline(input_file, s)) {
			line_tokens.clear();

			// Fill line_tokens with line tokens 
			TokenizeLine(s, line_tokens, ' ');
			ObjLineType line_type = GetLineType(line_tokens);

			switch (line_type) {
			case ObjLineType::VERTEX:
				output_file << s << std::endl;
				break;
			case ObjLineType::FACE:
				if (line_tokens.size() > 4) {
					TokenList face_1 = { line_tokens[0], line_tokens[1], line_tokens[2], line_tokens[3] };
					output_file << MergeTokenList(face_1) << std::endl;
					TokenList face_2 = { line_tokens[0], line_tokens[1], line_tokens[3], line_tokens[4] };
					output_file << MergeTokenList(face_2) << std::endl;
				}
				else {
					output_file << s << std::endl;
				}
				break;
			case ObjLineType::TEXTURE:
				output_file << s << std::endl;
				break;
			case ObjLineType::NORMAL:
				output_file << s << std::endl;
				break;

				// Handle unknown line types
			case ObjLineType::UNKNOWN:
			default:
				std::cout << "WARNING:: ObjReader found a problem on line " << line_number << " in file " << input_path << std::endl;
			}
			line_number++;
		}
	
	}

	static void SaveMesh(StaticMesh* mesh, const char* save_path) {
		std::ofstream file(save_path);
		if (!file.is_open()) {
			std::cout << "ERROR:: ObjReader::SaveMesh: Couldn't open file for writing!" << std::endl;
			return;
		}

		const auto& vertices = mesh->GetVertices();
		const auto& faces = mesh->GetFaces();

		// Write vertices
		for (const Vertex& vertex : vertices) {
			file << "v " << vertex.position.x << " " << vertex.position.y << " " << vertex.position.z << std::endl;
		}

		// Write normals
		for (const Vertex& vertex : vertices) {
			file << "vn " << vertex.normal.x << " " << vertex.normal.y << " " << vertex.normal.z << std::endl;
		}

		// Write texture coordinates
		for (const Vertex& vertex : vertices) {
			file << "vt " << vertex.uv.x << " " << vertex.uv.y << std::endl;
		}

		// Write faces
		for (const Face& face : faces) {
			file << "f " << face.a + 1 << "/" << face.a + 1 << "/" << face.a + 1 << " "
					     << face.b + 1 << "/" << face.b + 1 << "/" << face.b + 1 << " "
						 << face.c + 1 << "/" << face.c + 1 << "/" << face.c + 1 << std::endl;
		}

		file.close();
	}

	static void ReadZippedNaniteMesh(const std::string& file_path, std::vector<Vertex>& vertex_out, std::vector<std::vector<Face>>& cluster_face_out) {
		std::vector<vec3>    vertices;
		std::vector<vec3>    normals;
		std::vector<vec2>    tex_coords;
		std::vector<std::vector<ObjFace>> faces;
		std::ifstream file(file_path);

		std::string s;
		uint32_t line_number = 1;

		TokenList line_tokens;
		line_tokens.reserve(5);

		while (getline(file, s)) {
			line_tokens.clear();

			// Fill line_tokens with line tokens
			TokenizeLine(s, line_tokens, ' ');
			ObjLineType line_type = GetLineType(line_tokens);

			switch (line_type) {
			case ObjLineType::VERTEX:
				vertices.push_back(ObjReader::ParseVertex(line_tokens));
				break;
			case ObjLineType::FACE:
				if (line_tokens.size() > 4) {
					std::vector<ObjFace> faces_temp = ObjReader::ParseFaceList(line_tokens);
					for (ObjFace& face : faces_temp) {
						if (face.type != FaceType::UNKNOWN_FACE_TYPE) {
							faces.back().push_back(face);
						}
					}
				}
				else {
					ObjFace face = ObjReader::ParseFace(line_tokens, 0, 0, 0);
					if (face.type != FaceType::UNKNOWN_FACE_TYPE) {
						faces.back().push_back(face);
					}
				}
				break;
			case ObjLineType::TEXTURE:
				tex_coords.push_back(ObjReader::ParseTexCoord(line_tokens));
				break;
			case ObjLineType::NORMAL:
				normals.push_back(ObjReader::ParseNormal(line_tokens));
				break;
			case ObjLineType::CLUSTER_SEPARATOR:
				faces.push_back(std::vector<ObjFace>());
				break;
			// Handle unknown line types
			case ObjLineType::UNKNOWN:
			default:
				std::cout << "WARNING:: ObjReader found a problem on line " << line_number << " in file " << file_path << std::endl;
			}
			line_number++;
		}

		for (int i = 0; i < vertices.size(); i++) {
			Vertex v;
			v.position = vertices[i];
			v.normal = normals[i];
			v.uv = tex_coords[i];
			vertex_out.push_back(v);
		}

		for (const auto& cluster : faces) {
			std::vector<Face> cluster_faces;
			for (const auto& face : cluster) {
				Face f;
				f.a = face.v1 - 1;
				f.b = face.v2 - 1;
				f.c = face.v3 - 1;
				cluster_faces.push_back(f);
			}
			cluster_face_out.push_back(cluster_faces);
		}
	}
};