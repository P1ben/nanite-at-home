#pragma once
#include "framework.h"
#include "Face.h"
#include "Vertex.h"
#include "Mesh.h"
#include "VecLib/VecLib.h"

#define PAIR_HASH(a, b) a >= b ? (uint64_t)a * a + a + b : a + (uint64_t)b * b

class Edge {
public:
	VERTEX_ID first_vertex = ID_INVALID;
	VERTEX_ID second_vertex = ID_INVALID;
	bool deleted = false;


	Edge(VERTEX_ID _first, VERTEX_ID _second) {
		first_vertex = _first;
		second_vertex = _second;
	}

	Edge(const std::pair<VERTEX_ID, VERTEX_ID>& _vertices) {
		first_vertex = _vertices.first;
		second_vertex = _vertices.second;
	}

	float CalculateError(const std::vector<Face>& faces, const std::vector<Vertex>& vertices, const std::vector<std::vector<FACE_ID>>& vertex_faces) const {
		float error = 0;

		for (FACE_ID fid : vertex_faces[first_vertex]) {
			error += VecLib::CalculateSingleErrorByQEF(vertices[first_vertex].position, faces[fid].normal, vertices[first_vertex].position);
			error += VecLib::CalculateSingleErrorByQEF(vertices[second_vertex].position, faces[fid].normal, vertices[first_vertex].position);
		}

		for (FACE_ID fid : vertex_faces[second_vertex]) {
			error += VecLib::CalculateSingleErrorByQEF(vertices[first_vertex].position, faces[fid].normal, vertices[second_vertex].position);
			error += VecLib::CalculateSingleErrorByQEF(vertices[second_vertex].position, faces[fid].normal, vertices[second_vertex].position);
		}

		return error;
	}

	void Collapse(std::vector<Face>& faces, std::vector<Vertex>& vertices, std::vector<std::vector<FACE_ID>>& vertex_faces) {
		std::vector<vec3> normals;
		std::vector<vec3> points;

		int buffer_len = vertex_faces[first_vertex].size() + vertex_faces[second_vertex].size();
		normals.reserve(buffer_len);
		points.reserve(buffer_len);

		for (FACE_ID fid : vertex_faces[first_vertex]) {
			if (!faces[fid].deleted) {
				normals.push_back(faces[fid].normal);
				points.push_back(vertices[first_vertex].position);
			}
		}

		for (FACE_ID fid : vertex_faces[second_vertex]) {
			if (!faces[fid].deleted) {
				normals.push_back(faces[fid].normal);
				points.push_back(vertices[second_vertex].position);
				vertex_faces[first_vertex].push_back(fid);
				faces[fid].SwapAny(second_vertex, first_vertex);
				if (faces[fid].SearchDuplicate())
					faces[fid].deleted = true;
			}
		}

		// Set new vertex position
		vec3 new_point = VecLib::GetNewPointByQEF(normals, points);
		vertices[first_vertex].position = new_point;

		vec3 new_point_normal(0, 0, 0);
		int face_normal_counter = 0;

		// Recalculate face normals
		for (FACE_ID fid : vertex_faces[first_vertex]) {
			if (!faces[fid].deleted) {
				faces[fid].RecalculateNormal(vertices);
				new_point_normal += faces[fid].normal;
				face_normal_counter++;
			}
		}

		vertices[first_vertex].normal = new_point_normal / face_normal_counter;
		this->deleted = true;
	}

	static void SwapEndpoints(std::vector<Edge>& edges, VERTEX_ID old_id, VERTEX_ID new_id) {
		for (Edge& e : edges) {
			if (e.first_vertex == old_id) e.first_vertex = new_id;
			if (e.second_vertex == old_id) e.second_vertex = new_id;
		}
	}

	bool operator == (const Edge& b) const {
		return first_vertex == b.first_vertex && second_vertex == b.second_vertex;
	}

};

class Decimator {
private:
	std::vector<Vertex> vertices;
	std::vector<Face> faces;
	std::vector<Edge> edges;
	std::vector<float> edge_errors;
	std::vector<std::vector<FACE_ID>> vertex_faces;

public:
	Decimator(Mesh* mesh) {
		faces = std::vector<Face>(mesh->GetFaces());
		vertices = std::vector<Vertex>(mesh->GetVertices());
		vertex_faces.reserve(vertices.size());

		for (int i = 0; i < vertices.size(); i++) {
			vertex_faces.push_back(std::vector<FACE_ID>());
			vertex_faces[i].reserve(8);
		}

		std::set<uint64_t> hashed;

		for (FACE_ID i = 0; i < faces.size(); i++) {
			auto fe = faces[i].FirstEdge();
			auto se = faces[i].SecondEdge();
			auto te = faces[i].ThirdEdge();

			auto f_fe = hashed.find(PAIR_HASH(fe.first, fe.second));
			auto f_se = hashed.find(PAIR_HASH(se.first, se.second));
			auto f_te = hashed.find(PAIR_HASH(te.first, te.second));
			
			if (f_fe == hashed.end()) {
				edges.push_back(fe);
				hashed.insert(PAIR_HASH(fe.first, fe.second));
			}

			if (f_se == hashed.end()) {
				edges.push_back(se);
				hashed.insert(PAIR_HASH(se.first, se.second));
			}

			if (f_te == hashed.end()) {
				edges.push_back(te);
				hashed.insert(PAIR_HASH(te.first, te.second));
			}

			vertex_faces[faces[i].a].push_back(i);
			vertex_faces[faces[i].b].push_back(i);
			vertex_faces[faces[i].c].push_back(i);
		}

		edge_errors.reserve(edges.size());
		for (const Edge& edge : edges) {
			edge_errors.push_back(edge.CalculateError(faces, vertices, vertex_faces));
		}

		PRINT_TIME_TAKEN("Edge Collapsing:", {
			for (int i = 0; i < 1000; i++)
				DecimateEdge();
		})

		printf("Face count: %d\n", faces.size());
		printf("Vertex count: %d\n", vertices.size());
		printf("Edge count: %d\n", edges.size());
	}

	void DecimateEdge() {
		EDGE_ID min_id  = 0;
		float   min_val = 10000.0f;

		for (EDGE_ID i = 0; i < edge_errors.size(); i++) {
			if (!edges[i].deleted) {
				if (edge_errors[i] < min_val) {
					min_id = i;
					min_val = edge_errors[i];
				}
			}
		}

		edges[min_id].Collapse(faces, vertices, vertex_faces);
	}
};