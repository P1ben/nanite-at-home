#pragma once
#include "framework.h"
#include "Face.h"
#include "Mesh.h"
#include "VecLib/VecLib.h"
#include <igl/read_triangle_mesh.h>
#include <igl/list_to_matrix.h>
#include <igl/per_vertex_normals.h>
#include "Vertex.h"

class StaticMesh : public Mesh {
private:
	std::vector<Vertex> vertices;
	std::vector<Face> faces;

public:
	StaticMesh() {

	};

	StaticMesh(std::vector<Vertex>& _vertices, std::vector<Face>& _faces) {
		vertices = std::vector<Vertex>(_vertices);
		faces = std::vector<Face>(_faces);
		this->SetUpdated(true);
	};

	StaticMesh(std::string file_name) {
		RowMatrixf V;
		RowMatrixf N;
		RowMatrixu F;

		vec3 color;
		color.x = (float)rand() / RAND_MAX;
		color.y = (float)rand() / RAND_MAX;
		color.z = (float)rand() / RAND_MAX;

		igl::read_triangle_mesh(file_name, V, F);
		igl::per_vertex_normals(V, F, N);

		vertices.reserve(V.rows());
		faces.reserve(F.rows());

		for (int i = 0; i < V.rows(); i++) {
			//printf("\t%f %f %f\n", V(i, 0), V(i, 1), V(i, 2));
			vertices.push_back(Vertex(vec3(V(i, 0), V(i, 1), V(i, 2)), vec3(N(i, 0), N(i, 1), N(i, 2)), color));
		}

		for (int i = 0; i < F.rows(); i++) {
			faces.push_back(Face(F(i, 0), F(i, 1), F(i, 2), VecLib::CalculateFaceNormal(vertices[F(i, 0)].position, vertices[F(i, 1)].position, vertices[F(i, 2)].position)));
		}

		//for (const auto& vert : vertices) {
		//	printf("\t%f %f %f\n", vert.position.x, vert.position.y, vert.position.z);
		//}

		this->SetUpdated(true);

		printf("Number of vertices loaded: %u | Faces: %u\n", vertices.size(), faces.size());
	}

	void SetVertices(std::vector<Vertex>& vertices) {
		this->vertices = std::vector<Vertex>(vertices);
	}

	void SetFaces(std::vector<Face>& faces) {
		this->faces = std::vector<Face>(faces);
	}

	const std::vector<Vertex>& GetVertices() override {
		return vertices;
	}

	std::vector<Vertex>& GetVerticesEdit() {
		return vertices;
	}

	const std::vector<Face>& GetFaces() override {
		return faces;
	}

	int GetFaceCount() override {
		return faces.size();
	}

	float GetSurfaceArea() {
		float area = 0.0f;
		for (const auto& face : faces) {
			const auto& a_vert = vertices[face.a].position;
			const auto& b_vert = vertices[face.b].position;
			const auto& c_vert = vertices[face.c].position;

			float a = length(a_vert - b_vert);
			float b = length(a_vert - c_vert);
			float c = length(b_vert - c_vert);

			area += length(cross(a_vert - b_vert, a_vert - c_vert)) / 2;

			//area += (1.f / 4.f) * sqrtf((a + b - c) * (a - b + c) * (-a + b + c) * (a + b + c));
		}

		return area;
	}

	void Update(float center_distance_from_camera) override {
		return;
	}

	~StaticMesh() {
	}
};