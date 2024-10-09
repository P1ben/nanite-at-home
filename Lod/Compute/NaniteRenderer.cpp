#include "NaniteRenderer.h"
#include <glew.h>
#include "VertexBuffer.h"

#define INPUT_CLUSTER_BUFFER_BINDING 11
#define INPUT_METRIC_BUFFER_BINDING 12
#define INPUT_FACE_BUFFER_BINDING 13
#define OUTPUT_FACE_BUFFER_BINDING 14
#define OUTPUT_FACE_COUNT_BUFFER_BINDING 15
#define INPUT_VERTEX_BUFFER_BINDING 16
#define OUTPUT_VERTEX_BUFFER_BINDING 17
#define OUTPUT_VERTEX_COUNT_BUFFER_BINDING 18

NaniteRenderer* NaniteRenderer::instance = nullptr;

NaniteRenderer* NaniteRenderer::GetInstance() {
	if (instance == nullptr) {
		instance = new NaniteRenderer();
	}
	return instance;
}

void NaniteRenderer::FillBuffer(ClusterBuffer* i_cluster, FaceBuffer* i_faces, FaceBuffer* o_faces, float distance) {
	outputFaceCountBuffer->Reset();
	metricBuffer->Write(distance * distance / 25);


	i_cluster->Bind(INPUT_CLUSTER_BUFFER_BINDING);
	i_faces->Bind(INPUT_FACE_BUFFER_BINDING);
	metricBuffer->Bind(INPUT_METRIC_BUFFER_BINDING);
	o_faces->Bind(OUTPUT_FACE_BUFFER_BINDING);
	outputFaceCountBuffer->Bind(OUTPUT_FACE_COUNT_BUFFER_BINDING);

	counterShader->Use();
	counterShader->Dispatch(i_cluster->GetClusterCount(), 1, 1);

	uint32_t expected_face_count = outputFaceCountBuffer->GetVal();

	o_faces->Resize(expected_face_count);

	outputFaceCountBuffer->Reset();
	o_faces->Bind();

	computeShader->Use();
	computeShader->Dispatch(i_cluster->GetClusterCount(), 1, 1);

	//GLenum error;
	//while ((error = glGetError()) != GL_NO_ERROR) {
	//	std::cerr << "OpenGL Error: " << error << std::endl;
	//}
	//computeShader->Dispatch(2, 1, 1);
	o_faces->SetFaceCount(outputFaceCountBuffer->GetVal());

	//o_faces->Bind();
	//GLuint ssboSize;
	//glGetBufferParameteriv(GL_SHADER_STORAGE_BUFFER, GL_BUFFER_SIZE, (GLint*)&ssboSize);
	//std::cout << "SSBO Size: " << ssboSize << std::endl;

	//o_faces->Inspect();

	//o_faces->PrintData();
	//std::cout << "!!!!!!!!!!Face count: " << outputFaceCountBuffer->GetFaceCount() << std::endl;
}

void NaniteRenderer::FillBuffer(ClusterBuffer* i_cluster,
								FaceBuffer* i_faces,
								FaceBuffer* o_faces,
								VertexBuffer* i_verts,
								VertexBuffer* o_verts,
								float distance)
{
	outputFaceCountBuffer->Reset();
	outputVertexCountBuffer->Reset();
	metricBuffer->Write(distance * distance / 25);


	i_cluster->Bind(INPUT_CLUSTER_BUFFER_BINDING);
	metricBuffer->Bind(INPUT_METRIC_BUFFER_BINDING);
	i_faces->Bind(INPUT_FACE_BUFFER_BINDING);
	o_faces->Bind(OUTPUT_FACE_BUFFER_BINDING);
	outputFaceCountBuffer->Bind(OUTPUT_FACE_COUNT_BUFFER_BINDING);
	i_verts->Bind(INPUT_VERTEX_BUFFER_BINDING);
	o_verts->Bind(OUTPUT_VERTEX_BUFFER_BINDING);
	outputVertexCountBuffer->Bind(OUTPUT_VERTEX_COUNT_BUFFER_BINDING);

	counterShaderFull->Use();
	counterShaderFull->Dispatch(i_cluster->GetClusterCount(), 1, 1);

	uint32_t expected_face_count = outputFaceCountBuffer->GetVal();
	uint32_t expected_vertex_count = outputVertexCountBuffer->GetVal();

	o_faces->Resize(expected_face_count);
	o_verts->Resize(expected_vertex_count);

	outputFaceCountBuffer->Reset();
	outputVertexCountBuffer->Reset();

	computeShaderFull->Use();
	computeShaderFull->Dispatch(i_cluster->GetClusterCount(), 1, 1);

	//GLenum error;
	//while ((error = glGetError()) != GL_NO_ERROR) {
	//	std::cerr << "OpenGL Error: " << error << std::endl;
	//}
	//computeShader->Dispatch(2, 1, 1);
	o_faces->SetFaceCount(outputFaceCountBuffer->GetVal());

	//o_faces->Bind();
	//GLuint ssboSize;
	//glGetBufferParameteriv(GL_SHADER_STORAGE_BUFFER, GL_BUFFER_SIZE, (GLint*)&ssboSize);
	//std::cout << "SSBO Size: " << ssboSize << std::endl;

	//o_faces->Inspect();

	//o_faces->PrintData();
	//std::cout << "!!!!!!!!!!Face count: " << outputFaceCountBuffer->GetFaceCount() << std::endl;
}

void NaniteRenderer::Test() {
	outputFaceCountBuffer->Reset();
	outputFaceCountBuffer->Bind(OUTPUT_FACE_COUNT_BUFFER_BINDING);

	computeShader->Use();
	computeShader->Dispatch(10, 1, 1);
	//Sleep(5000);

	//std::cout << "!!!!!!!!!!Face count: " << outputFaceCountBuffer->GetFaceCount() << std::endl;
	printf("");
}
