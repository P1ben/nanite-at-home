#pragma once
#include "framework.h"
#include "Face.h"
#include "VecLib/VecLib.h"
#include <igl/read_triangle_mesh.h>
#include <igl/list_to_matrix.h>
#include <igl/per_vertex_normals.h>
#include "Vertex.h"
#include "Compute/FaceBuffer.h"
#include "Compute/VertexBuffer.h"

enum UpdateType {
	VERTEX_FACE_UPDATE,
	FACE_UPDATE,
	NO_UPDATE,
};

class Mesh {
private:
	UpdateType updated = NO_UPDATE;
public:

	const virtual std::vector<Vertex>& GetVertices() = 0;

	const virtual std::vector<Face>& GetFaces() = 0;

	virtual int GetFaceCount() = 0;

	UpdateType GetUpdated() {
		return updated;
	}

	void SetUpdated(UpdateType _upd) {
		updated = _upd;
	}

	virtual void Update(float center_distance_from_camera, FaceBuffer* o_faces, VertexBuffer* o_vertex) = 0;

	virtual ~Mesh() {
	}
};