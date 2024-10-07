#pragma once
#include "OMesh.h"

class StaticDecimator {
private:

	// Calculates squared error for a vertex
	static float CalculateSingleErrorQEF(OMesh& mesh, OMesh::VertexHandle point_vh, OMesh::FaceHandle fh, OMesh::VertexHandle point_on_plane_vh) {
		const OMesh::Point& p  = mesh.point(point_vh);
		const OMesh::Normal& n = mesh.normal(fh);
		const OMesh::Point& op = mesh.point(point_on_plane_vh);

		float error = n.dot(p - op);
		return error * error;
	}

	// Gets the best possible point for edge collapse
	static OMesh::Point GetNewPointByQEF2OMesh(OMesh& mesh, OMesh::VertexHandle vh0, OMesh::VertexHandle vh1, float weight) {
		using namespace OpenMesh;

		float BNX = 0;
		float BNY = 0;
		float BNZ = 0;

		float NXY = 0;
		float NXZ = 0;
		float NYZ = 0;

		float QX = 0;
		float QY = 0;
		float QZ = 0;

		for (OMesh::VertexFaceIter v0f_it(mesh.vf_iter(vh0)); v0f_it.is_valid(); ++v0f_it) {
			const auto& normal = mesh.normal(*v0f_it);
			const auto& point = mesh.point(vh0);
			float dot_prod = normal.dot(point);

			BNX += 2 * dot_prod * normal[0] + (2 * weight * point[0]);
			BNY += 2 * dot_prod * normal[1] + (2 * weight * point[1]);
			BNZ += 2 * dot_prod * normal[2] + (2 * weight * point[2]);

			NXY += 2 * normal[0] * normal[1];
			NXZ += 2 * normal[0] * normal[2];
			NYZ += 2 * normal[1] * normal[2];

			QX += 2 * normal[0] * normal[0] + (2 * weight);
			QY += 2 * normal[1] * normal[1] + (2 * weight);
			QZ += 2 * normal[2] * normal[2] + (2 * weight);
		}

		for (OMesh::VertexFaceIter v1f_it(mesh.vf_iter(vh1)); v1f_it.is_valid(); ++v1f_it) {
			const auto& normal = mesh.normal(*v1f_it);
			const auto& point = mesh.point(vh1);
			float dot_prod = normal.dot(point);

			BNX += 2 * dot_prod * normal[0] + (2 * weight * point[0]);
			BNY += 2 * dot_prod * normal[1] + (2 * weight * point[1]);
			BNZ += 2 * dot_prod * normal[2] + (2 * weight * point[2]);

			NXY += 2 * normal[0] * normal[1];
			NXZ += 2 * normal[0] * normal[2];
			NYZ += 2 * normal[1] * normal[2];

			QX += 2 * normal[0] * normal[0] + (2 * weight);
			QY += 2 * normal[1] * normal[1] + (2 * weight);
			QZ += 2 * normal[2] * normal[2] + (2 * weight);
		}

		float X = (QZ * (QY * QZ - NYZ * NYZ) * BNX - (QY * QZ - NYZ * NYZ) * BNZ * NXZ + (NXZ * NYZ - QZ * NXY) * QZ * BNY - (NXZ * NYZ - QZ * NXY) * NYZ * BNZ) / ((QY * QZ - NYZ * NYZ) * (QX * QZ - NXZ * NXZ) - (NXZ * NYZ - QZ * NXY) * (NYZ * NXZ - QZ * NXY));
		float Y = (QZ * BNY - NYZ * BNZ + (NYZ * NXZ - QZ * NXY) * X) / (QY * QZ - NYZ * NYZ);
		float Z = (BNZ - NXZ * X - NYZ * Y) / (QZ);

		return OMesh::Point(X, Y, Z);
	}

	// Checks if the removal of an edge interferes with mesh boundaries
	static bool CheckIfBoundary(OMesh& mesh, OMesh::EdgeHandle eh) {
		using namespace OpenMesh;

		HalfedgeHandle hh0 = mesh.halfedge_handle(eh, 0);
		HalfedgeHandle hh1 = mesh.halfedge_handle(eh, 1);

		return mesh.is_boundary(eh)
			|| mesh.is_boundary(mesh.to_vertex_handle(hh0))
			|| mesh.is_boundary(mesh.to_vertex_handle(hh1));
	}

	// Calculates squared error for a given edge
	static float CalculateErrorForEdge(OMesh& mesh, OMesh::EdgeHandle eh) {
		float error = 0.0f;

		OMesh::VertexHandle v0 = mesh.to_vertex_handle(mesh.halfedge_handle(eh, 0));
		OMesh::VertexHandle v1 = mesh.to_vertex_handle(mesh.halfedge_handle(eh, 1));

		for (OMesh::VertexFaceIter v0f_it(mesh.vf_begin(v0)); v0f_it.is_valid(); ++v0f_it) {
			error += CalculateSingleErrorQEF(mesh, v1, *v0f_it, v0);
		}

		for (OMesh::VertexFaceIter v1f_it(mesh.vf_begin(v1)); v1f_it.is_valid(); ++v1f_it) {
			error += CalculateSingleErrorQEF(mesh, v0, *v1f_it, v1);
		}

		return error;
	}

public:

	// Simplifies cluster, removes triagles so that the triangle count decreases to max_ratio of original
	// Returns: introduced error sum for cluster
	static float DecimateCluster(OMesh& cluster_mesh, float max_ratio) {
		using namespace OpenMesh;
		
		cluster_mesh.request_edge_status();
		cluster_mesh.request_halfedge_status();
		cluster_mesh.request_vertex_status();
		cluster_mesh.request_face_status();

		// Calculate error values for edges
		for (auto edge : cluster_mesh.edges()) {
			float error = CalculateErrorForEdge(cluster_mesh, edge);

			cluster_mesh.data(edge).set_error(error);
		}

		int face_count = cluster_mesh.n_faces();
		int max_faces_to_remove = face_count * (1 - max_ratio) + 1;

		const auto compare_fun = [&](EdgeHandle _lhs, EdgeHandle _rhs) {
			return cluster_mesh.data(_lhs).error() < cluster_mesh.data(_rhs).error();
			};

		std::set <EdgeHandle, decltype(compare_fun)> edges_set(compare_fun);

		for (OMesh::EdgeIter edg_it(cluster_mesh.edges_begin()); edg_it != cluster_mesh.edges_end(); ++edg_it) {
			if (!CheckIfBoundary(cluster_mesh, *edg_it))
				edges_set.insert(*edg_it);
		}

		int deleted_count = 0;
		float introduced_error = 0.f;

		while (edges_set.size() != 0 && deleted_count < max_faces_to_remove) {
			EdgeHandle to_be_deleted = *edges_set.begin();
			edges_set.erase(edges_set.begin());

			if (cluster_mesh.is_collapse_ok(cluster_mesh.halfedge_handle(to_be_deleted, 0))) {
				HalfedgeHandle hh = cluster_mesh.halfedge_handle(to_be_deleted, 0);
				VertexHandle   rem_vh = cluster_mesh.to_vertex_handle(hh);
				VertexHandle   del_vh = cluster_mesh.to_vertex_handle(cluster_mesh.opposite_halfedge_handle(hh));

				OMesh::Point new_point     = GetNewPointByQEF2OMesh(cluster_mesh, rem_vh, del_vh, 0.002f);
				OMesh::Point old_point     = cluster_mesh.point(rem_vh);
				OMesh::Point deleted_point = cluster_mesh.point(del_vh);

				float ratio = ((new_point - old_point).length() / (deleted_point - old_point).length());

				OMesh::TexCoord2D rem_tex = cluster_mesh.texcoord2D(rem_vh);
				OMesh::TexCoord2D del_tex = cluster_mesh.texcoord2D(del_vh);

				OMesh::TexCoord2D new_tex = rem_tex + (del_tex - rem_tex) * ratio;
				//OMesh::TexCoord2D new_tex = del_tex;

				cluster_mesh.set_point(rem_vh, new_point);
				cluster_mesh.set_texcoord2D(rem_vh, new_tex);
				introduced_error += cluster_mesh.data(cluster_mesh.edge_handle(hh)).error();
				cluster_mesh.collapse(hh);

				//for (OMesh::VertexFaceIter vfi(cluster_mesh.vf_begin(rem_vh)); vfi.is_valid(); ++vfi) {
				//	cluster_mesh.set_normal(*vfi, cluster_mesh.calc_face_normal(*vfi));
				//}

				for (OMesh::VertexEdgeIter vei(cluster_mesh.ve_begin(rem_vh)); vei.is_valid(); ++vei) {
					edges_set.erase(*vei);
					if (!CheckIfBoundary(cluster_mesh, *vei))
					{
						cluster_mesh.data(*vei).set_error(CalculateErrorForEdge(cluster_mesh, *vei));
						edges_set.insert(*vei);
					}
				}
				deleted_count += 2;
			}
		}
		cluster_mesh.garbage_collection();
		std::cout << "Remaining faces: " << cluster_mesh.n_faces() << std::endl;
		std::cout << "Stop cause: " << (edges_set.size() == 0 ? "No more edges to remove" : "Max faces reached") << std::endl;
		return introduced_error;
	}

	// Decimates cluster faces until a given facecount is reached
	// Returns: introduced error sum
	static float DecimateClusterUntil(OMesh& cluster_mesh, uint32_t max_triangle_count) {
		int   triangle_count   = cluster_mesh.n_faces();
		float decimation_ratio = (float)max_triangle_count / triangle_count;

		return DecimateCluster(cluster_mesh, decimation_ratio);
	}
};