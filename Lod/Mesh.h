#pragma once
#include "framework.h"
#include "Face.h"
#include "VecLib/VecLib.h"
#include <igl/read_triangle_mesh.h>
#include <igl/list_to_matrix.h>
#include <igl/per_vertex_normals.h>
#include "Vertex.h"

class Mesh {
private:
	bool updated = false;
public:
	const virtual std::vector<Vertex>& GetVertices() = 0;

	const virtual std::vector<Face>& GetFaces() = 0;

	virtual int GetFaceCount() = 0;

	bool GetUpdated() {
		return updated;
	}

	void SetUpdated(bool _upd) {
		updated = _upd;
	}

	virtual void Update(float center_distance_from_camera) = 0;

	~Mesh() {
	}
};