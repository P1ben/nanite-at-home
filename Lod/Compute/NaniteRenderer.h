#pragma once
#include "ClusterBuffer.h"
#include "ComputeShader.h"
#include "FaceBuffer.h"
#include "VertexBuffer.h"
#include "OutputFaceCountBuffer.h"
#include "FloatBuffer.h"

class NaniteRenderer {
private:
	ComputeShader*         computeShader;
	ComputeShader*         computeShaderFull;
	ComputeShader*         counterShader;
	ComputeShader*         counterShaderFull;

	UintBuffer*            outputFaceCountBuffer;
	UintBuffer*            outputVertexCountBuffer;
	FloatBuffer*           metricBuffer;

	static NaniteRenderer* instance;

	NaniteRenderer() {
		computeShader           = new ComputeShader("shaders/nanite_compute-cs.glsl");
		counterShader           = new ComputeShader("shaders/nanite_compute_counter-cs.glsl");
		counterShaderFull       = new ComputeShader("shaders/nanite_compute_counter_full-cs.glsl");
		computeShaderFull       = new ComputeShader("shaders/nanite_compute_full-cs.glsl");

		outputFaceCountBuffer   = new UintBuffer();
		outputVertexCountBuffer = new UintBuffer();
		metricBuffer            = new FloatBuffer();
	}

public:
	NaniteRenderer(NaniteRenderer& _) = delete;
	void operator=(const NaniteRenderer& _) = delete;

	static NaniteRenderer* GetInstance();

	void FillBuffer(ClusterBuffer* i_cluster, FaceBuffer* i_faces, FaceBuffer* output_face_buffer, float distance);
	void FillBuffer(ClusterBuffer* i_cluster,
					FaceBuffer* i_faces,
					FaceBuffer* o_faces,
					VertexBuffer* i_verts,
					VertexBuffer* o_verts,
					float distance);
	void Test();
};