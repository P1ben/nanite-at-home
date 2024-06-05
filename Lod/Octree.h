#pragma once
#include "framework.h"
#include "Mesh.h"
#include <igl/per_face_normals.h>
#include "ThreadPool/ThreadPool.h"
#include "ThreadPool/Task.h"

enum Algorithm {
	ALG_SIMPLE_AVG,
	ALG_QEF
};

struct VertexData {
	VERTEX_ID vertex;
	uint32_t index;
	std::vector<int> faceIndices;
	std::vector<Face>* faceBuffer;
	std::vector<Vertex>* vertexBuffer;

	VertexData(VERTEX_ID _vertex, uint32_t _index, std::vector<Vertex>* _vertexBuffer, std::vector<Face>* _faceBuffer) {
		vertex = _vertex;
		index = _index;
		faceBuffer = _faceBuffer;
		vertexBuffer = _vertexBuffer;
		faceIndices.reserve(4);
	}

	static VertexData GetRepresentative(std::vector<VertexData>& vertices, Algorithm algorithm) {
		uint32_t lowest_index = vertices[0].index;
		int normals_len = 0;

		for (const VertexData& vd : vertices) {
			normals_len += vd.faceIndices.size();
		}

		std::vector<vec3> normals;
		std::vector<vec3> points;

		normals.reserve(normals_len);
		points.reserve(vertices.size());

		VertexData new_vertex(vertices[0].vertex, lowest_index, vertices[0].vertexBuffer, vertices[0].faceBuffer);
		for (int i = 0; i < vertices.size(); i++) {
			for (auto j : vertices[i].faceIndices) {
				normals.push_back((*vertices[i].faceBuffer)[j].normal);
				points.push_back((*vertices[0].vertexBuffer)[vertices[i].vertex].position);
				(*vertices[i].faceBuffer)[j].SwapAny(vertices[i].index, lowest_index);
				if (!(*vertices[i].faceBuffer)[j].SearchDuplicate()) {
					new_vertex.faceIndices.push_back(j);
				}
			}
		}
		if (algorithm == ALG_QEF) {
			// -- QEF --//
			(*vertices[0].vertexBuffer)[new_vertex.vertex].position = VecLib::GetNewPointByQEF2(normals, points, 0.0003f);
			// !- QEF -!//
		
		}
		else {
			//new_vertex.vertex = VecLib::GetNewPointByQEF2(normals, points, 0.0f);
			// -- Average -- //
			vec3 avg = vec3(0, 0, 0);
			for (const VertexData& vec : vertices) {
				avg = avg + (*vertices[0].vertexBuffer)[vec.vertex].position;
			}
			avg = avg / vertices.size();
			(*vertices[0].vertexBuffer)[new_vertex.vertex].position = avg;
			//new_vertex.vertex = vec3(0, 0, 0);
			// !- Average -! //
		}
		(*vertices[0].vertexBuffer)[new_vertex.vertex].normal = VecLib::GetAverage(normals);
		return new_vertex;
	}

	std::vector<std::vector<VertexData>> GroupMeshPart(const std::vector<VertexData>& vertices) {
	
	}

	// Used
	static void CalculateAndSetRepresentative(const std::vector<VertexData>& vertices, Algorithm algorithm) {
		uint32_t lowest_index = vertices[0].index;
		int normals_len = 0;

		for (const VertexData& vd : vertices) {
			normals_len += vd.faceIndices.size();
		}

		std::vector<vec3> normals;
		std::vector<vec3> points;

		normals.reserve(normals_len);
		points.reserve(vertices.size());

		for (int i = 0; i < vertices.size(); i++) {
			for (auto j : vertices[i].faceIndices) {
				normals.push_back((*vertices[i].faceBuffer)[j].normal);
				points.push_back((*vertices[0].vertexBuffer)[vertices[i].vertex].position);
				(*vertices[i].faceBuffer)[j].SwapAny(vertices[i].index, lowest_index);
			}
		}
		if (algorithm == ALG_QEF) {
			// -- QEF --//
			(*vertices[0].vertexBuffer)[lowest_index].position = VecLib::GetNewPointByQEF2(normals, points, 0.05f);
			// !- QEF -!//

		}
		else {
			//new_vertex.vertex = VecLib::GetNewPointByQEF2(normals, points, 0.0f);
			// -- Average -- //
			vec3 avg = vec3(0, 0, 0);
			for (const VertexData& vec : vertices) {
				avg = avg + (*vertices[0].vertexBuffer)[vec.vertex].position;
			}
			avg = avg / vertices.size();
			(*vertices[0].vertexBuffer)[lowest_index].position = avg;
			//new_vertex.vertex = vec3(0, 0, 0);
			// !- Average -! //
		}
		(*vertices[0].vertexBuffer)[lowest_index].normal = VecLib::GetAverage(normals);
	}

	static void CalculateAndSetRepresentativeAsync(void* args) {
		std::pair<std::vector<VertexData>, Algorithm>* argswt = static_cast<std::pair<std::vector<VertexData>, Algorithm>*>(args);
		const std::vector<VertexData>& vertices = argswt->first;
		Algorithm algorithm = argswt->second;

		uint32_t lowest_index = vertices[0].index;
		int normals_len = 0;

		for (const VertexData& vd : vertices) {
			normals_len += vd.faceIndices.size();
		}

		std::vector<vec3> normals;
		std::vector<vec3> points;

		normals.reserve(normals_len);
		points.reserve(vertices.size());

		for (int i = 0; i < vertices.size(); i++) {
			for (auto j : vertices[i].faceIndices) {
				normals.push_back((*vertices[i].faceBuffer)[j].normal);
				points.push_back((*vertices[0].vertexBuffer)[vertices[i].vertex].position);
				(*vertices[i].faceBuffer)[j].SwapAny(vertices[i].index, lowest_index);
			}
		}
		if (algorithm == ALG_QEF) {
			// -- QEF --//
			(*vertices[0].vertexBuffer)[lowest_index].position = VecLib::GetNewPointByQEF2(normals, points, 0.0003f);
			// !- QEF -!//

		}
		else {
			//new_vertex.vertex = VecLib::GetNewPointByQEF2(normals, points, 0.0f);
			// -- Average -- //
			vec3 avg = vec3(0, 0, 0);
			for (const VertexData& vec : vertices) {
				avg = avg + (*vertices[0].vertexBuffer)[vec.vertex].position;
			}
			avg = avg / vertices.size();
			(*vertices[0].vertexBuffer)[lowest_index].position = avg;
			//new_vertex.vertex = vec3(0, 0, 0);
			// !- Average -! //
		}
		(*vertices[0].vertexBuffer)[lowest_index].normal = VecLib::GetAverage(normals);
	}
};

struct OctNode {
	std::vector<VertexData> vertices;
	std::vector<OctNode*> leaves;
	vec3  center = vec3(0, 0, 0);
	float radius = 1.0f;
	int depth = 0;


	OctNode() {}
	OctNode(vec3 _center, float _radius, int _depth) {
		//this->vertices = RowMatrixf(vertices);
		radius = _radius;
		center = _center;
		depth = _depth;
		leaves.reserve(8);
		vertices.reserve(10);
	}

	OctNode* GetNewBoundingBoxNode(const vec3& vertex, const vec3& old_center, float old_radius, int depth) {
		std::vector<vec3> options;

		options.push_back(center + ((radius / 2) * vec3( 1,  1, 1)));
		options.push_back(center + ((radius / 2) * vec3( 1, -1, 1)));
		options.push_back(center + ((radius / 2) * vec3(-1,  1, 1)));
		options.push_back(center + ((radius / 2) * vec3(-1, -1, 1)));

		options.push_back(center + ((radius / 2) * vec3( 1,  1, -1)));
		options.push_back(center + ((radius / 2) * vec3( 1, -1, -1)));
		options.push_back(center + ((radius / 2) * vec3(-1,  1, -1)));
		options.push_back(center + ((radius / 2) * vec3(-1, -1, -1)));

		for (const vec3& c : options) {
			if (IsVertexInBox(vertex, c, radius / 2)) {
				return new OctNode(c, radius / 2, depth);
			}
		}
		return nullptr;
	}

	bool AddVertex(const VertexData& vertex, uint32_t level) {
		if (level == 0) {
			this->vertices.push_back(vertex);
			return true;
		}

		for (OctNode* node : leaves) {
			if (node->IsVertexInBox((*vertex.vertexBuffer)[vertex.vertex].position)) {
				return node->AddVertex(vertex, level - 1);
			}
		}

		OctNode* new_node = GetNewBoundingBoxNode((*vertex.vertexBuffer)[vertex.vertex].position, center, radius, depth + 1);
		if (new_node) {
			this->leaves.push_back(new_node);
			return new_node->AddVertex(vertex, level - 1);
		}
		//printf("Couldn't find suitable position in tree for vertex [x: %f, y: %f, z: %f]\n", vertex.x, vertex.y, vertex.z);
		return false;
	}

	void PrintLeafVertexCount() {
		if (leaves.empty()) {
			if (vertices.size() > 0)
				//printf("Vertices contained: %d\n", vertices.size());
			return;
		}
		for (auto node : leaves) {
			node->PrintLeafVertexCount();
		}
	}

	size_t GetContainedVertexCount() {
		int count = 0;
		if (leaves.empty()) {
			return vertices.size();
		}
		for (auto node : leaves) {
			count += node->GetContainedVertexCount();
		}
		return count;
	}

	int GetLeafCount() {
		float count = 0;
		if (leaves.empty()) {
			return 1;
		}
		for (auto node : leaves) {
			count += node->GetLeafCount();
		}
		return count;
	}

	int GetNodeCount() {
		float count = 0;
		if (leaves.empty()) {
			return 1;
		}
		for (auto node : leaves) {
			count += node->GetNodeCount();
		}
		return count + 1;
	}

	void GetLeafNodes(std::vector<OctNode*>& leaf_nodes) {
		if (leaves.empty()) leaf_nodes.push_back(this);
		else {
			for (auto l : leaves) {
				l->GetLeafNodes(leaf_nodes);
			}
		}
	}

	void Destroy() {
		if (leaves.empty()) delete this;
		else {
			for (auto l : leaves) {
				l->Destroy();
			}
			delete this;
		}
	}

	void GetLeafNodesData(std::vector<VertexData>& leaf_nodes) {
		if (leaves.empty()) {
			for (const VertexData& a : this->vertices)
				leaf_nodes.push_back(a);
		}
		else {
			for (auto l : leaves) {
				l->GetLeafNodesData(leaf_nodes);
			}
		}
	}

	void GetCalcVerticesAtDepth(std::vector<VertexData>& data_out, int depth, Algorithm algorithm) {
		if (this->depth == depth) {
			std::vector<VertexData> children;
			this->GetLeafNodesData(children);
			//printf("LeafNodeDataLen: %d\n", children.size());
			data_out.push_back(VertexData::GetRepresentative(children, algorithm));
		}
		else {
			for (auto l : leaves) {
				l->GetCalcVerticesAtDepth(data_out, depth, algorithm);
			}
		}
	}

	void PerformSimplificationAtDepth(int depth, Algorithm algorithm) {
		if (this->depth == depth) {
			std::vector<VertexData> children;
			this->GetLeafNodesData(children);
			VertexData::CalculateAndSetRepresentative(children, algorithm);
		}
		else {
			for (auto l : leaves) {
				l->PerformSimplificationAtDepth(depth, algorithm);
			}
		}
	}

	void PerformSimplificationAtDepthAsync(int depth, Algorithm algorithm, ThreadPool& thread_pool) {
		if (this->depth == depth) {
			std::vector<VertexData> children;
			this->GetLeafNodesData(children);
			thread_pool.QueueJob(VertexData::CalculateAndSetRepresentativeAsync, std::pair<std::vector<VertexData>, Algorithm>(children, algorithm));
			//VertexData::GetRepresentative(children, algorithm);
		}
		else {
			for (auto l : leaves) {
				l->PerformSimplificationAtDepthAsync(depth, algorithm, thread_pool);
			}
		}
	}

	bool IsVertexInBox(const vec3& vertex) {
		return IsVertexInBox(vertex, this->center, this->radius);
	}

	bool IsVertexInBox(const vec3& vertex, const vec3& center, float radius) {
		if (!(vertex.x >= center.x - radius && vertex.x <= center.x + radius)) return false;
		if (!(vertex.y >= center.y - radius && vertex.y <= center.y + radius)) return false;
		if (!(vertex.z >= center.z - radius && vertex.z <= center.z + radius)) return false;
		return true;
	}
};

class Octree {
	OctNode* head;
	vec3  center = vec3(0, 0, 0);
	float radius = 1.0f;
	uint32_t depth;
	std::vector<Vertex> vertices;
	std::vector<Vertex> vertices_temp_buffer;
	std::vector<Face> faces;
	std::vector<Face> faces_temp_buffer;

public:
	Octree(Mesh* mesh,  vec3 center, float radius, uint32_t depth) {
		const auto & V = mesh->GetVertices();
		const auto & F = mesh->GetFaces();

		this->depth = depth;
		head = new OctNode(center, radius, 0);
		int fail = 0;

		std::vector<VertexData> dataPredef;
		dataPredef.reserve(V.size());

		faces = std::vector<Face>(F);

		vertices = std::vector<Vertex>(V);
		vertices_temp_buffer = std::vector<Vertex>(V);
		faces_temp_buffer = std::vector<Face>(F);


		for (int i = 0; i < vertices.size(); i++) {
			dataPredef.push_back(VertexData(i, i, &vertices_temp_buffer, &faces_temp_buffer));
		}

		for (int i = 0; i < faces.size(); i++) {
			const Face& face = faces[i];
			dataPredef[face.a].faceIndices.push_back(i);
			dataPredef[face.b].faceIndices.push_back(i);
			dataPredef[face.c].faceIndices.push_back(i);
		}

		for (auto& dt : dataPredef) {
			if (!head->AddVertex(dt, depth))
				fail++;
		}

		//printf("Octree loaded successfully (Misses: %d) (Vertices: %u)\n", fail, head->GetContainedVertexCount());
		//printf("Leaf count: %u\n", head->GetLeafCount());
		//printf("Node count: %u\n", head->GetNodeCount());
		//PrintVertexCountByLeaf();
	}

	void PerformSimplification() {
		faces_temp_buffer = std::vector<Face>(faces);
		printf("\tPerforming simplification\n");
		printf("Before vertice count: %d\n", head->GetContainedVertexCount());
		//head->EliminateVertices();
		printf("After vertice count: %d\n", head->GetContainedVertexCount());
	}

	StaticMesh* RestoreMesh(int dpth, Algorithm algorithm) {
		StaticMesh* ret_mesh;
		faces_temp_buffer = std::vector<Face>(faces);
		vertices_temp_buffer = std::vector<Vertex>(vertices);
		//std::vector<VertexData> vdt;
		//std::vector<OctNode*> leaves;
		//head->GetLeafNodes(leaves);

		//head->GetCalcVerticesAtDepth(vdt, dpth, algorithm);
		head->PerformSimplificationAtDepth(dpth, algorithm);

		//ThreadPool thread_pool;
		//thread_pool.Start();

		//head->PerformSimplificationAtDepthAsync(dpth, algorithm, thread_pool);

		//thread_pool.Join();

		//std::vector<Vertex> vertices = std::vector<Vertex>(vertices_temp_buffer);
		//vertices.reserve(vdt.size());

		//for (int i = 0; i < vdt.size(); i++) {
		//	if (vdt[i].index != i) {
		//		for (auto j : vdt[i].faceIndices) {
		//			(*vdt[i].faceBuffer)[j].SwapAny(vdt[i].index, i);
		//		}
		//		vdt[i].index = i;
		//	}
		//}

		/*for (auto vd : vdt) {
			Face::UpdateIndices(faces, vd.old_indices, vd.index);
		}*/

		/*for (auto& fc : faces) {
			for (const auto& vd : vdt) {
				for (auto ind : vd.old_indices) {
					fc.SwapAny(ind, vd.index);
				}
			}
		}*/

		std::vector<Face> rem_faces;

		for (const Face& f : faces_temp_buffer) {
			//printf("Face: (%u, %u, %u)\n", f.a, f.b, f.c);
			if (!f.SearchDuplicate()) {
				rem_faces.push_back(f);
				//printf("Face: (%u, %u, %u)\n", f.a, f.b, f.c);
			}
		}

		printf("New vertex count: %d, face count: %d\n", vertices.size(), rem_faces.size());
		ret_mesh = new StaticMesh(vertices_temp_buffer, rem_faces);
		return ret_mesh;

	}

	static StaticMesh* SimplifyMesh(Mesh* input, Algorithm algo, float epsilon) {
		float mesh_radius = VecLib::GetBoundingBoxRadius(input->GetVertices()) + 0.01f;
		int   depth = 0;
		float temp = epsilon;

		while (mesh_radius > temp) {
			depth += 1;
			temp  *= 2;
		}

		vec3 center = VecLib::GetAverage(input->GetVertices());

		Mesh* res;

		Octree temp_tree = Octree(input, center, temp, depth);
		return temp_tree.RestoreMesh(depth, algo);
	}

	static Task<Mesh>* SimplifyMeshAsync(Mesh* input, Algorithm algo, float epsilon) {
		Task<Mesh>* ret_task = new Task<Mesh>();
		ret_task->StartJob(
			new std::thread([=] {
				Mesh* new_mesh = SimplifyMesh(input, algo, epsilon);
				ret_task->_SetResult(new_mesh);
			})
		);
		return ret_task;
	}

	void PrintVertexCountByLeaf() {
		head->PrintLeafVertexCount();
	}

	~Octree() {
		head->Destroy();
	}
};