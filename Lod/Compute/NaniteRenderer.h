#pragma once
#include "ClusterBuffer.h"
#include "ComputeShader.h"
#include "FaceBuffer.h"
#include "OutputFaceCountBuffer.h"
#include "FloatBuffer.h"

class NaniteRenderer {
private:
	ComputeShader*         computeShader;
	ComputeShader*         counterShader;
	OutputFaceCountBuffer* outputFaceCountBuffer;
	FloatBuffer*           metricBuffer;

	static NaniteRenderer* instance;

	NaniteRenderer() {
		computeShader         = new ComputeShader("shaders/nanite_compute.glsl");
		counterShader         = new ComputeShader("shaders/nanite_compute_count.glsl");
		outputFaceCountBuffer = new OutputFaceCountBuffer();
		metricBuffer          = new FloatBuffer();
	}

public:
	NaniteRenderer(NaniteRenderer& _) = delete;
	void operator=(const NaniteRenderer& _) = delete;

	static NaniteRenderer* GetInstance();

	void FillBuffer(ClusterBuffer* i_cluster, FaceBuffer* i_faces, FaceBuffer* output_face_buffer, float distance);
	void Test();
};