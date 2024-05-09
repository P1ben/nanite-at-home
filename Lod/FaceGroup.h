#pragma once
#include "framework.h"
#include "EdgeCollection.h"
#include <list>
#include <exception>

class FaceGroup {
	uint32_t level;

	FACE_GROUP_ID id;
	bool is_parent = true;

	std::vector<FACE_ID> faces;
	std::set<FACE_GROUP_ID> neighbours;
	std::set<EDGE_ID> outer_edges;
	std::set<EDGE_ID> inside_edges;
	std::vector<VERTEX_ID> inside_vertices;
	EdgeCollection* collection;

public:
	FaceGroup(FACE_ID face, EdgeCollection* _collection) {
		faces.push_back(face);
		id = face;
		collection = _collection;

		const auto neighs = collection->GetNeighbours(face);
		if (std::get<0>(neighs) != ID_INVALID)
			neighbours.insert(std::get<0>(neighs));

		if (std::get<1>(neighs) != ID_INVALID)
			neighbours.insert(std::get<1>(neighs));

		if (std::get<2>(neighs) != ID_INVALID)
			neighbours.insert(std::get<2>(neighs));

		const auto& edg = collection->GetFaceEdges(face);

		outer_edges.insert(std::get<0>(edg));
		outer_edges.insert(std::get<1>(edg));
		outer_edges.insert(std::get<2>(edg));

		level = 0;
	}

	size_t GetFaceCount() const {
		return faces.size();
	}

	FACE_GROUP_ID FindMergeableNeighbour(const std::vector<FaceGroup>& face_groups) {
		std::pair<FACE_GROUP_ID, size_t> counter = {ID_INVALID, ID_INVALID};
		for (FACE_GROUP_ID group : neighbours) {
			if (face_groups[group].faces.size() <= counter.second) {
				counter.first = group;
				counter.second = face_groups[group].faces.size();
			}
		}
		return counter.first;
	}

	EDGE_ID FindConnectingEdge(const FaceGroup& other) {
		for (auto ooe : other.outer_edges) {
			if (outer_edges.find(ooe) != outer_edges.end()) {
				return ooe;
			}
		}
		return ID_INVALID;
	}

	bool IsParent() const {
		return is_parent;
	}

	void PrintInfo() const {
		printf("-- Face group info --\n");
		printf("\tID: %d\n", id);
		printf("\tLevel: %d\n", level);
		printf("\tFaces contained: %d\n", faces.size());
		printf("\tInside vertices contained: %d\n", inside_vertices.size());
		printf("\tInside edges contained: %d\n", inside_edges.size());
		printf("\tOutside edges contained: %d\n", outer_edges.size());
		printf("\tNeighbour count: %d\n", neighbours.size());
		printf("-! Face group info !-\n");
	}

	void ChangeNeighbour(FACE_GROUP_ID from, FACE_GROUP_ID to) {
		neighbours.erase(from);
		neighbours.insert(to);
	}

	void NotifyNeighbours(std::vector<FaceGroup>& face_groups, FACE_GROUP_ID to) {
		for (const FACE_GROUP_ID neig : neighbours) {
			face_groups[neig].ChangeNeighbour(id, to);
		}
	}

	void Merge(FaceGroup& other, std::map<FACE_ID, FACE_GROUP_ID>& face_facegroup_map, std::vector<FaceGroup>& face_groups) {
		EDGE_ID connecting_edge = FindConnectingEdge(other);

		if (connecting_edge == ID_INVALID) {
			printf("Couldn't find connecting edge");
			throw std::invalid_argument("Couldn't find connecting edge");
		}

		const auto& conn_edge_faces = collection->GetEdgeFaces(connecting_edge);

		FACE_ID conn_face;
		if (face_facegroup_map[conn_edge_faces.first] == id)
			conn_face = conn_edge_faces.second;
		else
			conn_face = conn_edge_faces.first;

		int conn_face_id = std::distance(other.faces.begin(), std::find(other.faces.begin(), other.faces.end(), conn_face));

		for (int i = conn_face_id; i < other.faces.size(); i++) {
			AddFace(other.faces[i]);
			face_facegroup_map[other.faces[i]] = id;
		}

		for (int i = conn_face_id - 1; i >= 0; i--) {
			AddFace(other.faces[i]);
			face_facegroup_map[other.faces[i]] = id;
		}


		this->neighbours.erase(other.id);
		other.NotifyNeighbours(face_groups, id);

		this->neighbours.insert(other.neighbours.begin(), other.neighbours.end());
		this->neighbours.erase(id);

		level += 1;
		other.is_parent = false;
	}

	void AddFace(FACE_ID face) {
		const auto& fedges = collection->GetFaceEdges(face);

		EDGE_ID a = std::get<0>(fedges);
		EDGE_ID b = std::get<1>(fedges);
		EDGE_ID c = std::get<2>(fedges);

		bool a_in = outer_edges.find(a) != outer_edges.end();
		bool b_in = outer_edges.find(b) != outer_edges.end();
		bool c_in = outer_edges.find(c) != outer_edges.end();

		if (a_in && b_in) {
			inside_vertices.push_back(collection->FindCommonPoint(a, b));
		}
		else if (a_in && c_in) {
			inside_vertices.push_back(collection->FindCommonPoint(a, c));
		}
		else if (b_in && c_in) {
			inside_vertices.push_back(collection->FindCommonPoint(b, c));
		}

		faces.push_back(face);

		if (a_in) {
			inside_edges.insert(a);
			outer_edges.erase(a);
		}
		else {
			outer_edges.insert(a);
		}

		if (b_in) {
			inside_edges.insert(b);
			outer_edges.erase(b);
		}
		else {
			outer_edges.insert(b);
		}

		if (c_in) {
			inside_edges.insert(c);
			outer_edges.erase(c);
		}
		else {
			outer_edges.insert(c);
		}
	}
};