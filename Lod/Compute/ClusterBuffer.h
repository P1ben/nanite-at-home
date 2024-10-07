#pragma once
#include "SSBO.h"

struct ClusterData {
	float parental_cutoff;
	float self_cutoff;

	uint32_t face_start;
	uint32_t faces_length;
};

class ClusterBuffer {
private:
	SSBO* ssbo;
	uint32_t cluster_count;

public:
	ClusterBuffer(const std::vector<ClusterData>& clusters) {
		cluster_count = clusters.size();
		ssbo = new SSBO(cluster_count * sizeof(ClusterData), (void*)clusters.data());
	}

	void Bind(uint32_t binding) {
		ssbo->Bind(binding);
	}

	uint32_t GetClusterCount() {
		return cluster_count;
	}

	~ClusterBuffer() {
		delete ssbo;
	}
};