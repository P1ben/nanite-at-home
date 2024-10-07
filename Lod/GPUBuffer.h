#pragma once

#include "framework.h"
#include "Face.h"
#include "Mesh.h"
#include "Compute/FaceBuffer.h"
#include "Compute/VertexBuffer.h"

class GPUBuffer {
private:
	uint32_t vao;
	//uint32_t vbo;
	// uint32_t ebo;
	FaceBuffer* faceBuffer = nullptr;
	VertexBuffer* vertexBuffer = nullptr;

	//uint32_t number_of_faces;

public:
	GPUBuffer() {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		//glGenBuffers(1, &vbo);
		vertexBuffer = new VertexBuffer();


		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->GetId());
		/*glGenBuffers(1, &ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);*/
		// Position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// Normal
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)sizeof(vec3));
		glEnableVertexAttribArray(1);

		// Color
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(2 * sizeof(vec3)));
		glEnableVertexAttribArray(2);

		// UV
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(vec3)));
		glEnableVertexAttribArray(3);
		//number_of_faces = 0;
		faceBuffer = new FaceBuffer(0);
		//glVertexArrayElementBuffer(vao, faceBuffer->GetId());
	}

	void Fill(const std::vector<Vertex>& vertices, const std::vector<Face>& faces) {
		//std::vector<unsigned> faces_list;
		//faces_list.reserve(faces.size());
		//for (const Face& f : faces) {
		//	faces_list.push_back(f.a);
		//	faces_list.push_back(f.b);
		//	faces_list.push_back(f.c);
		//}

		//glBindBuffer(GL_ARRAY_BUFFER, vbo);
		//glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
		vertexBuffer->ManualFill(vertices);
		faceBuffer->ManualFill(faces);
		/*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(uint32_t) * 3, faces.data(), GL_STATIC_DRAW);*/
		//number_of_faces = faces.size();
		//printf("Buffer loaded, vertices: %d | faces: %d\n", vertices.size(), faces.size());
	}

	void FillFaces(const std::vector<Face>& faces) {
		//glBindBuffer(GL_ARRAY_BUFFER, vbo);
		faceBuffer->ManualFill(faces);
		faceBuffer->Bind();
		//faceBuffer->Inspect();
		//GLuint ssboSize;
		//glGetBufferParameteriv(GL_SHADER_STORAGE_BUFFER, GL_BUFFER_SIZE, (GLint*)&ssboSize);
		//std::cout << "SSBO Size: " << ssboSize << std::endl;
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(uint32_t) * 3, faces.data(), GL_STATIC_DRAW);
		//number_of_faces = faces.size();
		//printf("Buffer loaded, faces: %d\n", faces.size());
	}

	// Do not use, broken
	void Fill(const RowMatrixf& V, const RowMatrixf& N, const RowMatrixu& F) {
		std::vector<vec3> bufferData;
		int rows = V.rows();
		for (int i = 0; i < rows; i++) {
			bufferData.push_back(vec3(V(i, 0), V(i, 1), V(i, 2)));
			bufferData.push_back(vec3(N(i, 0), N(i, 1), N(i, 2)));
			/*bufferData.push_back(V(i));
			bufferData.push_back(N(i));*/
		}

		//glBindBuffer(GL_ARRAY_BUFFER, vbo);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, F.size() * sizeof(unsigned), F.data(), GL_STATIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, (V.size() + N.size()) * sizeof(float), bufferData.data(), GL_STATIC_DRAW);
		//number_of_faces = F.rows();
	}

	void Fill(Mesh* mesh) {
		glBindVertexArray(vao);
		this->Fill(mesh->GetVertices(), mesh->GetFaces());
	}

	void FillFacesOnly(Mesh* mesh) {
		glBindVertexArray(vao);
		this->FillFaces(mesh->GetFaces());
	}

	FaceBuffer* GetFaceBuffer() {
		return faceBuffer;
	}

	uint32_t GetFaceCount() {
		return faceBuffer->GetFaceCount();
	}

	uint32_t GetVertexCount() {
		return vertexBuffer->GetVertexCount();
	}

	void Draw() {
		glBindVertexArray(vao);
		/*glBindBuffer(GL_ARRAY_BUFFER, vbo); */
		vertexBuffer->BindAsVertexBuffer();
		faceBuffer->BindAsIndexBuffer();
		//faceBuffer->Inspect();
		//faceBuffer->PrintData();
		//glDrawArrays(GL_TRIANGLES, 0, faceBuffer->GetFaceCount() * 3);
		glDrawElements(GL_TRIANGLES, faceBuffer->GetFaceCount() * 3, GL_UNSIGNED_INT, nullptr);
		//GLenum error;
		//while ((error = glGetError()) != GL_NO_ERROR) {
		//	std::cerr << "OpenGL Error: " << error << std::endl;
		//}
	}

	~GPUBuffer() {
		//glDeleteBuffers(1, &vbo);
		//glDeleteBuffers(1, &ebo);
		glDeleteVertexArrays(1, &vao);
		delete faceBuffer;
		delete vertexBuffer;
	}
};