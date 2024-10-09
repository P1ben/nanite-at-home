#pragma once
#include "SSBO.h"

class UintBuffer {
private:
	SSBO* ssbo;

public:
	UintBuffer() {
		ssbo = new SSBO(sizeof(uint32_t));
	}

	void Bind(uint32_t binding) {
		ssbo->Bind(binding);
	}

	void Bind() {
		ssbo->Bind();
	}

	uint32_t GetVal() {
		uint32_t return_val;
		ssbo->ReadData(0, sizeof(uint32_t), &return_val);
		return return_val;
	}

	void Reset() {
		uint32_t zero = 0;
		ssbo->WriteData(0, sizeof(uint32_t), &zero);
	}

	~UintBuffer() {
		delete ssbo;
	}
};