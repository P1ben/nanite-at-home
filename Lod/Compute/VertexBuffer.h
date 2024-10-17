#pragma once
#include "SSBO.h"
#include <vector>
#include "../Vertex.h"
#include <iostream>

class VertexBuffer {
private:
	SSBO* ssbo;
	uint32_t vertex_count;

public:
	VertexBuffer() {
		vertex_count = 0;
		ssbo = new SSBO(0);
	}

	void Bind(uint32_t binding) {
		ssbo->Bind(binding);
	}

	void BindAsVertexBuffer() {
		//ssbo->Bind(0);
		ssbo->BindAsVertexBuffer();
	}

	void ManualFill(const std::vector<Vertex>& vertices) {
		ssbo->ManualVertexFill(vertices);
		vertex_count = vertices.size();
	}

	void Resize(uint32_t vertex_count) {
		ssbo->Resize(vertex_count * sizeof(Vertex));
		vertex_count = vertex_count;
	}

	uint32_t GetVertexCount() {
		return vertex_count;
	}

	//void PrintData() {
	//	uint32_t* data = new uint32_t[vertex_count * 3];
	//	ssbo->ReadData(0, vertex_count * 3 * sizeof(uint32_t), data);
	//	for (int i = 0; i < face_count; i += 3) {
	//		std::cout << data[i] << " " << data[i + 1] << " " << data[i + 2] << std::endl;
	//	}
	//	delete[] data;
	//}

	uint32_t GetId() {
		return ssbo->GetId();
	}

	~VertexBuffer() {
		delete ssbo;
	}
};