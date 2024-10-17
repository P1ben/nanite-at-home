#pragma once
#include "SSBO.h"
#include <vector>
#include "../Face.h"
#include <iostream>

class FaceBuffer {
private:
	SSBO* ssbo;
	uint32_t max_face_count = 0;
	uint32_t face_count = 0;

public:
	FaceBuffer(uint32_t _max_face_count) {
		max_face_count = _max_face_count;
		ssbo = new SSBO(max_face_count * sizeof(Face));
	}

	void Resize(uint32_t face_count) {
		ssbo->Resize(face_count * sizeof(Face));
		max_face_count = face_count;
	}

	void Bind(uint32_t binding) {
		ssbo->Bind(binding);
	}

	void BindAsIndexBuffer() {
		//ssbo->Bind(1);
		ssbo->BindAsIndexBuffer();
	}

	void Bind() {
		ssbo->Bind();
	}

	void ManualFill(const std::vector<Face>& faces) {
		ssbo->ManualIndexFill(faces);
		face_count = faces.size();
		max_face_count = faces.size();
	}

	uint32_t GetMaxFaceCount() {
		return max_face_count;
	}

	uint32_t GetFaceCount() {
		return face_count;
	}

	void SetFaceCount(uint32_t _face_count) {
		face_count = _face_count;
	}

	void Inspect() {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo->GetId());

		void* ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		if (ptr) {
			uint32_t* data = static_cast<uint32_t*>(ptr);

			for (size_t i = 0; i < this->face_count * 3 && i < 5 * 3; ++i) {
				std::cout << "Index " << i << ": " << data[i] << std::endl;
			}

			// Step 3: Unmap the buffer when done
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		}
		else {
			std::cerr << "Failed to map buffer!" << std::endl;
		}
	}

	void PrintData() {
		uint32_t* data = new uint32_t[face_count * 3];
		ssbo->ReadData(0, face_count * 3 * sizeof(uint32_t), data);
		for (int i = 0; i < face_count; i += 3) {
			std::cout << data[i] << " " << data[i + 1] << " " << data[i + 2] << std::endl;
		}
		delete[] data;
	}

	uint32_t GetId() {
		return ssbo->GetId();
	}

	~FaceBuffer() {
		delete ssbo;
	}
};