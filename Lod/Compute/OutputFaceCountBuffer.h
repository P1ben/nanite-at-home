#pragma once
#include "SSBO.h"

class OutputFaceCountBuffer {
private:
	SSBO* ssbo;

public:
	OutputFaceCountBuffer() {
		ssbo = new SSBO(sizeof(uint32_t));
	}

	void Bind(uint32_t binding) {
		ssbo->Bind(binding);
	}

	void Bind() {
		ssbo->Bind();
	}

	uint32_t GetFaceCount() {
		uint32_t return_val;
		ssbo->ReadData(0, sizeof(uint32_t), &return_val);
		return return_val;
	}

	void Reset() {
		uint32_t zero = 0;
		ssbo->WriteData(0, sizeof(uint32_t), &zero);
	}

	~OutputFaceCountBuffer() {
		delete ssbo;
	}
};