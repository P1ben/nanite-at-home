#pragma once
#include "framework.h"
#include <map>
#include "Face.h"


class EdgeCollection {
	std::map<std::pair<VERTEX_ID, VERTEX_ID>, EDGE_ID>				vertices_to_edge;
	std::map<EDGE_ID, std::pair<FACE_ID, FACE_ID>>					edge_faces;
	std::map<EDGE_ID, std::pair<VERTEX_ID, VERTEX_ID>>				edge_vertices;
	std::map<FACE_ID, std::tuple<EDGE_ID, EDGE_ID, EDGE_ID>>		face_edges;
	std::map<FACE_ID, std::tuple<VERTEX_ID, VERTEX_ID, VERTEX_ID>>  face_vertices;

public:
	EdgeCollection() {}
	EdgeCollection(const std::vector<Face>& faces) {
		EDGE_ID edges_contained = 0;
		for (int i = 0; i < faces.size(); i++) {
			std::pair<VERTEX_ID, VERTEX_ID> first = faces[i].FirstEdge();
			std::pair<VERTEX_ID, VERTEX_ID> second = faces[i].SecondEdge();
			std::pair<VERTEX_ID, VERTEX_ID> third = faces[i].ThirdEdge();
			std::tuple<EDGE_ID, EDGE_ID, EDGE_ID> face_entry;

			// First edge
			if (vertices_to_edge.find(first) == vertices_to_edge.end()) {
				vertices_to_edge.insert({first, edges_contained});
				edge_faces.insert({edges_contained, std::pair<FACE_ID, FACE_ID>(i, ID_INVALID)});
				edge_vertices.insert({edges_contained, first});
				std::get<0>(face_entry) = edges_contained;
				edges_contained++;
			}
			else {
				EDGE_ID id = vertices_to_edge[first];
				edge_faces[id].second = i;
				std::get<0>(face_entry) = id;
			}

			// Second edge
			if (vertices_to_edge.find(second) == vertices_to_edge.end()) {
				vertices_to_edge.insert({ second, edges_contained });
				edge_faces.insert({ edges_contained, std::pair<FACE_ID, FACE_ID>(i, ID_INVALID) });
				edge_vertices.insert({ edges_contained, second });
				std::get<1>(face_entry) = edges_contained;
				edges_contained++;
			}
			else {
				EDGE_ID id = vertices_to_edge[second];
				edge_faces[id].second = i;
				std::get<1>(face_entry) = id;
			}

			// Third edge
			if (vertices_to_edge.find(third) == vertices_to_edge.end()) {
				vertices_to_edge.insert({ third, edges_contained });
				edge_faces.insert({ edges_contained, std::pair<FACE_ID, FACE_ID>(i, ID_INVALID) });
				edge_vertices.insert({ edges_contained, third });
				std::get<2>(face_entry) = edges_contained;
				edges_contained++;
			}
			else {
				EDGE_ID id = vertices_to_edge[third];
				edge_faces[id].second = i;
				std::get<2>(face_entry) = id;
			}

			face_edges.insert({i, face_entry});
			face_vertices.insert({i, std::tuple<VERTEX_ID, VERTEX_ID, VERTEX_ID>(faces[i].a, faces[i].b, faces[i].c)});
		}

		//int counter = 0;
		//for (auto& e : edge_faces) {
		//	if (e.second.second == ID_INVALID) {
		//		printf("%d %d\n", e.second.first, e.second.second);
		//		counter++;
		//	}
		//}
		//printf("Bad entries: %d\n", counter);
	}

	void PrintInfo() {
		printf("----- EdgeCollection Data -----\n");
		printf("\tFace count: %u\n", face_edges.size());
		printf("\tEdge count: %u\n", vertices_to_edge.size());
		//printf("\t\n");
		//printf("\t\n");
	}

	std::tuple<FACE_ID, FACE_ID, FACE_ID> GetNeighbours(FACE_ID face) {
		const auto& edges = face_edges[face];
		const auto& neig_0 = edge_faces[std::get<0>(edges)];
		const auto& neig_1 = edge_faces[std::get<1>(edges)];
		const auto& neig_2 = edge_faces[std::get<2>(edges)];

		return std::tuple<FACE_ID, FACE_ID, FACE_ID>(
			neig_0.first == face ? neig_0.second : neig_0.first,
			neig_1.first == face ? neig_1.second : neig_1.first,
			neig_2.first == face ? neig_2.second : neig_2.first
		);
	}

	std::tuple<EDGE_ID, EDGE_ID, EDGE_ID>& GetFaceEdges(FACE_ID face) {
		return face_edges[face];
	}

	std::pair<FACE_ID, FACE_ID>& GetEdgeFaces(EDGE_ID edge) {
		return edge_faces[edge];
	}

	VERTEX_ID FindCommonPoint(FACE_ID a, FACE_ID b, FACE_ID c) {
		const auto& a_v = face_vertices[a];
		const auto& b_v = face_vertices[b];
		const auto& c_v = face_vertices[c];

		std::vector<VERTEX_ID> vec;
		vec.reserve(3 * 3);
		vec.push_back(std::get<0>(a_v));
		vec.push_back(std::get<1>(a_v));
		vec.push_back(std::get<2>(a_v));
		vec.push_back(std::get<0>(b_v));
		vec.push_back(std::get<1>(b_v));
		vec.push_back(std::get<2>(b_v));
		vec.push_back(std::get<0>(c_v));
		vec.push_back(std::get<1>(c_v));
		vec.push_back(std::get<2>(c_v));

		std::sort(vec.begin(), vec.end());

		int counter = 0;
		VERTEX_ID curr_val = 0;

		for (auto v : vec) {
			if (v != curr_val) {
				counter = 0;
				curr_val = v;
			}
			counter++;
			if (counter == 3) {
				return curr_val;
			}
		}
		return 0U;
	}

	VERTEX_ID FindCommonPoint(EDGE_ID a, EDGE_ID b) {
		const auto& a_v = edge_vertices[a];
		const auto& b_v = edge_vertices[b];

		if (a_v.first == b_v.first) return a_v.first;
		if (a_v.second == b_v.second) return a_v.second;
		return 0U;
	}
};