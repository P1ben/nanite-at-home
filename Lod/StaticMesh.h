#pragma once
#include "framework.h"
#include "Face.h"
#include "Mesh.h"
#include "VecLib/VecLib.h"
#include <igl/read_triangle_mesh.h>
#include <igl/list_to_matrix.h>
#include <igl/per_vertex_normals.h>
#include <igl/readOBJ.h>
#include "Vertex.h"
#include "OMesh.h"

class StaticMesh : public Mesh {
private:
	std::vector<Vertex> vertices;
	std::vector<Face> faces;

	// TODO: This is a preliminary function to read meshes with OpenMesh. It is not yet fully implemented.
	void _ReadWithOMeshPRELIMINARY(std::string file_name) {
		OMesh mesh;
		mesh.request_vertex_normals();
		mesh.request_vertex_texcoords2D();

		if (!OpenMesh::IO::read_mesh(mesh, file_name))
		{
			printf("ERROR::StaticMesh:: Couldn't read mesh with _ReadWithOMeshPRELIMINARY!");
			exit(1);
		}

		mesh.update_vertex_normals();

		vec3 color;
		color.x = (float)rand() / RAND_MAX;
		color.y = (float)rand() / RAND_MAX;
		color.z = (float)rand() / RAND_MAX;

		for (OMesh::VertexIter v_i(mesh.vertices_begin()); v_i != mesh.vertices_end(); ++v_i) {
			auto& position = mesh.point(*v_i);
			auto& normal   = mesh.normal(*v_i);
			auto& uv       = mesh.texcoord2D(*v_i);

			vec3 pos_v3 = vec3(position[0], position[1], position[2]);
			vec3 norm_v3 = vec3(normal[0], normal[1], normal[2]);
			vec2 uv_v2 = vec2(uv[0], uv[1]);

			Vertex temp = Vertex(pos_v3, norm_v3, color, uv_v2);

			vertices.push_back(temp);
		}

		for (OMesh::FaceIter f_i(mesh.faces_begin()); f_i != mesh.faces_end(); ++f_i) {
			Face face;
			int counter = 0;
			for (OMesh::FaceVertexIter fv_i(mesh.fv_begin(*f_i)); fv_i != mesh.fv_end(*f_i); ++fv_i) {
				if (counter == 0) {
					face.a = fv_i->idx();
				}
				else if (counter == 1) {
					face.b = fv_i->idx();
				}
				else if (counter == 2) {
					face.c = fv_i->idx();
					break;
				}
				counter++;
			}

			faces.push_back(face);
		}

	}

public:
	StaticMesh() {

	};

	StaticMesh(std::vector<Vertex>& _vertices, std::vector<Face>& _faces) {
		vertices = std::vector<Vertex>(_vertices);
		faces = std::vector<Face>(_faces);
		this->SetUpdated(true);
	};

	StaticMesh(const StaticMesh& other) {
		vertices = std::vector<Vertex>(other.vertices);
		faces = std::vector<Face>(other.faces);
		this->SetUpdated(true);
	}

	StaticMesh(std::string file_name, bool use_omesh_preliminary = false) {
		if (use_omesh_preliminary) {
			_ReadWithOMeshPRELIMINARY(file_name);
			return;
		}

		RowMatrixf V;
		RowMatrixf N;
		RowMatrixu F;

		RowMatrixf UV;

		RowMatrixf FTC;
		RowMatrixf FN;

		vec3 color;
		color.x = (float)rand() / RAND_MAX;
		color.y = (float)rand() / RAND_MAX;
		color.z = (float)rand() / RAND_MAX;

		//igl::read_triangle_mesh(file_name, V, F);
		igl::readOBJ(file_name, V, UV, N, F, FTC, FN);
		igl::per_vertex_normals(V, F, N);

		vertices.reserve(V.rows());
		faces.reserve(F.rows());

		std::cout << "Mesh data: " << V.rows() << " vertices, " << F.rows() << " faces " << UV.rows() << " textcoords" << std::endl;

		// If there are UV coordinates, load them as well
		if (UV.rows() > 0) {
			for (int i = 0; i < V.rows(); i++) {
				//printf("\t%f %f %f\n", V(i, 0), V(i, 1), V(i, 2));
				vertices.push_back(Vertex(vec3(V(i, 0), V(i, 1), V(i, 2)), vec3(N(i, 0), N(i, 1), N(i, 2)), color, vec2(UV(i, 0), UV(i, 1))));
			}
		}
		else {
			for (int i = 0; i < V.rows(); i++) {
				//printf("\t%f %f %f\n", V(i, 0), V(i, 1), V(i, 2));
				vertices.push_back(Vertex(vec3(V(i, 0), V(i, 1), V(i, 2)), vec3(N(i, 0), N(i, 1), N(i, 2)), color));
			}
		}

		for (int i = 0; i < F.rows(); i++) {
			//faces.push_back(Face(F(i, 0), F(i, 1), F(i, 2), VecLib::CalculateFaceNormal(vertices[F(i, 0)].position, vertices[F(i, 1)].position, vertices[F(i, 2)].position)));
			faces.push_back(Face(F(i, 0), F(i, 1), F(i, 2)));
		}

		//for (const auto& vert : vertices) {
		//	printf("\t%f %f %f\n", vert.position.x, vert.position.y, vert.position.z);
		//}

		this->SetUpdated(true);

		printf("Number of vertices loaded: %u | Faces: %u\n", vertices.size(), faces.size());
	}

	void AddVertex(Vertex& vert) {
		this->vertices.push_back(vert);
	}

	void AddFace(Face& face) {
		this->faces.push_back(face);
	}

	void SetVertices(std::vector<Vertex>& vertices) {
		this->vertices = std::vector<Vertex>(vertices);
	}

	void SetVertexColor(const vec3& color) {
		for (Vertex& vert : vertices) {
			vert.color = color;
		}
	}

	void SetRandomVertexColor() {
		vec3 color = vec3((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX);
		for (Vertex& vert : vertices) {
			vert.color = color;
		}
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

	int GetVertexFaceCount(VERTEX_ID vertex) {
		int counter = 0;
		
		for (Face& face : faces) {
			if (face.ContainsId(vertex)) {
				counter++;
			}
		}

		return counter;
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