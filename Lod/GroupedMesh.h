#pragma once

#include "framework.h"
#include "FaceGroup.h"
#include "Mesh.h"

class GroupedMesh {
	std::vector<Face> faces;
	std::vector<Vertex> vertices;
	std::vector<FaceGroup> face_groups;
	EdgeCollection collection;
	std::map<FACE_ID, FACE_GROUP_ID> face_facegroup_map;

public:
	GroupedMesh(Mesh* mesh) {
		const auto& V = mesh->GetVertices();
		const auto& F = mesh->GetFaces();

		faces = std::vector<Face>(F);
		vertices = std::vector<Vertex>(V);

		collection = EdgeCollection(faces);
		
		face_groups.reserve(faces.size());

		for (int i = 0; i < faces.size(); i++) {
			face_groups.push_back(FaceGroup(i, &collection));
			face_facegroup_map[i] = i;
		}
	}

	void PerformMergeCycle() {
		for (auto& group : face_groups) {
			if (group.IsParent()) {
				FACE_GROUP_ID neig = group.FindMergeableNeighbour(face_groups);
				if (neig != ID_INVALID)
					group.Merge(face_groups[neig], face_facegroup_map, face_groups);
			}
		}
	}

	void PrintInfoVerbose() {
		int group_count = 0;
		size_t faces_in_parents = 0;
		for (const auto& group : face_groups) {
			if (group.IsParent()) {
				group.PrintInfo();
				group_count++;
				faces_in_parents += group.GetFaceCount();
			}
		}
		printf("!Total number of parent groups: %d!\n", group_count);
		printf("!Total number of faces in parents: %d!\n", faces_in_parents);
	}
};