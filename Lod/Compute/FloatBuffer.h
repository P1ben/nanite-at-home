#pragma once
#include "SSBO.h"

class FloatBuffer {
private:
	SSBO* ssbo;

public:
	FloatBuffer() {
		ssbo = new SSBO(sizeof(float));
	}

	void Bind(uint32_t binding) {
		ssbo->Bind(binding);
	}

	uint32_t GetFaceCount() {
		uint32_t return_val;
		ssbo->ReadData(0, sizeof(float), &return_val);
		return return_val;
	}

	void Write(float value) {
		ssbo->WriteData(0, sizeof(float), &value);
	}

	~FloatBuffer() {
		delete ssbo;
	}
};