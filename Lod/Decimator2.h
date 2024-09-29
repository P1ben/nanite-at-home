#pragma once

#include <iostream>
#include "OMesh.h"
#include "framework.h"
#include "Mesh.h"
#include <list>
#include <queue>
#include "GraphPartitioner.h"
#include "Nanite/NaniteMesh.h"
#include "StaticDecimator.h"
#include "Framebuffer/Framebuffer.h"

class Decimator2 {
	OMesh mesh;
	float error_avg = 0.0f;

	float CalculateSingleErrorQEF(OMesh::VertexHandle point_vh, OMesh::FaceHandle fh, OMesh::VertexHandle point_on_plane_vh) {
		const OMesh::Point& p = mesh.point(point_vh);
		const OMesh::Normal& n = mesh.normal(fh);
		const OMesh::Point& op = mesh.point(point_on_plane_vh);

		float error = n.dot(p - op);
		return error * error;
	}

	OMesh::Point GetNewPointByQEF2OMesh(OMesh::VertexHandle vh0, OMesh::VertexHandle vh1, float weight) {
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
			const auto& point  = mesh.point(vh0);
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

	float CalculateErrorForEdge(OMesh::EdgeHandle eh) {
		float error = 0.0f;

		OMesh::VertexHandle v0 = mesh.to_vertex_handle(mesh.halfedge_handle(eh, 0));
		OMesh::VertexHandle v1 = mesh.to_vertex_handle(mesh.halfedge_handle(eh, 1));

		for (OMesh::VertexFaceIter v0f_it(mesh.vf_begin(v0)); v0f_it.is_valid(); ++v0f_it) {
			error += CalculateSingleErrorQEF(v1, *v0f_it, v0);
		}

		for (OMesh::VertexFaceIter v1f_it(mesh.vf_begin(v1)); v1f_it.is_valid(); ++v1f_it) {
			error += CalculateSingleErrorQEF(v0, *v1f_it, v1);
		}

		return error;
	}

	bool CompareEdges(OMesh::EdgeHandle _lhs, OMesh::EdgeHandle _rhs) {
		return mesh.data(_lhs).error() < mesh.data(_rhs).error();
	}

	OMesh::VertexHandle CopyVertexNoDupl(const Vertex& vertex) {
		OMesh::Point      new_vert = OMesh::Point(vertex.position.x, vertex.position.y, vertex.position.z);
		OMesh::TexCoord2D new_tex = OMesh::TexCoord2D(vertex.uv.x, vertex.uv.y);
		for (OMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); ++v_it) {
			if (mesh.point(*v_it) == new_vert && mesh.texcoord2D(*v_it) == new_tex)
				return *v_it;
		}

		OMesh::Normal new_normal = OMesh::Normal(vertex.normal.x, vertex.normal.y, vertex.normal.z);

		OMesh::VertexHandle vh_n = mesh.add_vertex(new_vert);
		mesh.set_normal(vh_n, new_normal);
		mesh.set_texcoord2D(vh_n, new_tex);

		return vh_n;
	}

public:
	// TODO: Generates mesh, not good for performance
	Decimator2() {}

	Decimator2(const char* input_path) {
		OpenMesh::IO::Options ropt;
		ropt += OpenMesh::IO::Options::VertexNormal;
		ropt += OpenMesh::IO::Options::VertexTexCoord;

		mesh.request_vertex_normals();
		mesh.request_face_normals();
		mesh.request_vertex_texcoords2D();

		if (!OpenMesh::IO::read_mesh(mesh, input_path, ropt)) {
			std::cerr << "Cannot read mesh from file 'input.obj'" << std::endl;
		}

		mesh.update_face_normals();
		mesh.update_vertex_normals();

		int edge_count = 0;
		for (auto edge : mesh.edges()) {
			float error = CalculateErrorForEdge(edge);

			mesh.data(edge).set_error(error);
			//edge_count++;
			//error_avg += error;
		}
		//error_avg /= edge_count;
	
	}

	Decimator2(Mesh* _mesh) {
		mesh.request_vertex_normals();
		mesh.request_face_normals();
		mesh.request_vertex_texcoords2D();

		const auto& vertices = _mesh->GetVertices();
		const auto& faces = _mesh->GetFaces();

		std::vector<OMesh::VertexHandle> vhandles;
		vhandles.reserve(vertices.size());

		for (const Vertex& vert : vertices) {
			OMesh::VertexHandle vh = mesh.add_vertex(OMesh::Point(vert.position.x, vert.position.y, vert.position.z));
			mesh.set_normal(vh, MeshTraits::Normal(vert.normal.x, vert.normal.y, vert.normal.z));
			mesh.set_texcoord2D(vh, MeshTraits::TexCoord2D(vert.uv.x, vert.uv.y));
			vhandles.push_back(vh);
		}

		for (const Face& face : faces) {
			OMesh::FaceHandle fh = mesh.add_face(vhandles[face.a], vhandles[face.b], vhandles[face.c]);
			//mesh.set_normal(fh, MeshTraits::Normal(face.normal.x, face.normal.y, face.normal.z));
		}

		//for (const Face& face : faces) {
		//	OMesh::VertexHandle vh0 = CopyVertexNoDupl(vertices[face.a]);
		//	OMesh::VertexHandle vh1 = CopyVertexNoDupl(vertices[face.b]);
		//	OMesh::VertexHandle vh2 = CopyVertexNoDupl(vertices[face.c]);

		//	OMesh::FaceHandle fh = mesh.add_face(vh0, vh1, vh2);
		//	mesh.set_normal(fh, MeshTraits::Normal(face.normal.x, face.normal.y, face.normal.z));
		//}

		mesh.update_face_normals();
		mesh.update_vertex_normals();

		int edge_count = 0;
		for (auto edge : mesh.edges()) {
			float error = CalculateErrorForEdge(edge);

			mesh.data(edge).set_error(error);
			//edge_count++;
			//error_avg += error;
		}
		//error_avg /= edge_count;
	}

	void MetisTest() {
		(void)GraphPartitioner::PartitionMesh(mesh, 128);
	}

	static void CreateNaniteMesh(std::string input_mesh_path, std::string output_mesh_path) {
		//humanoid_mesh = new StaticMesh(input_mesh_path);
		Decimator2 dm2(ObjReader::ReadObj(input_mesh_path.c_str()));

		StaticMesh* temp_mesh = dm2.ConvertToStaticMesh();
		Framebuffer::CreateObjNormalMap(temp_mesh, (output_mesh_path + "\\normals.jpg").c_str());
		//lod_meshes.push_back(temp_mesh);
		delete temp_mesh;

		NaniteMesh* nanite_mesh;
		PRINT_TIME_TAKEN("Creating Nanite Mesh:", {
			nanite_mesh = dm2.GetNaniteMesh();
		})

		PRINT_TIME_TAKEN("Generating Nanite Mesh:", {
			nanite_mesh->Generate();
		})

		nanite_mesh->WriteClusterDetailsIntoFile(std::string("logs\\log.txt"));

		PRINT_TIME_TAKEN("Saving Nanite Mesh:", {
			nanite_mesh->Save(output_mesh_path);
		})

		delete nanite_mesh;
	}

	NaniteMesh* GetNaniteMesh() {
		return new NaniteMesh(mesh);
	}

	StaticMesh* ConvertToStaticMesh() {
		std::vector<Vertex> vertices;
		std::vector<Face>   faces;

		for (OMesh::VertexIter v_i(mesh.vertices_begin()); v_i != mesh.vertices_end(); ++v_i) {
			auto& position = mesh.point(*v_i);
			auto& normal = mesh.normal(*v_i);
			auto& uv = mesh.texcoord2D(*v_i);

			vec3 pos_v3 = vec3(position[0], position[1], position[2]);
			vec3 norm_v3 = vec3(normal[0], normal[1], normal[2]);
			vec2 uv_v2 = vec2(uv[0], uv[1]);

			Vertex temp = Vertex(pos_v3, norm_v3, vec3(1.0f), uv_v2);

			vertices.push_back(temp);
		}

		for (OMesh::FaceIter f_i(mesh.faces_begin()); f_i != mesh.faces_end(); ++f_i) {
			Face face;
			int counter = 0;
			for (OMesh::FaceVertexIter fv_i(mesh.fv_begin(*f_i)); fv_i != mesh.fv_end(*f_i); ++fv_i) {
				if (counter == 0) {
					face.a = fv_i->idx();
				}
				else if (counter == 1) {
					face.b = fv_i->idx();
				}
				else if (counter == 2) {
					face.c = fv_i->idx();
					break;
				}
				counter++;
			}

			faces.push_back(face);
		}

		return new StaticMesh(vertices, faces);
	
	}

	void CollapseEdge(OMesh::EdgeHandle eh) {
		using namespace OpenMesh;

		if (mesh.is_collapse_ok(mesh.halfedge_handle(eh, 0))) {
			HalfedgeHandle hh = mesh.halfedge_handle(eh, 0);
			VertexHandle   rem_vh = mesh.to_vertex_handle(hh);
			VertexHandle   del_vh = mesh.to_vertex_handle(mesh.opposite_halfedge_handle(hh));

			mesh.set_point(rem_vh, GetNewPointByQEF2OMesh(rem_vh, del_vh, 0.0f));
			mesh.collapse(hh);

			for (OMesh::VertexFaceIter vfi(mesh.vf_begin(rem_vh)); vfi.is_valid(); ++vfi) {
				mesh.set_normal(*vfi, mesh.calc_face_normal(*vfi));
			}

			for (OMesh::VertexEdgeIter vei(mesh.ve_begin(rem_vh)); vei.is_valid(); ++vei) {
				mesh.data(*vei).set_error(CalculateErrorForEdge(*vei));
			}
		}
	}

	bool CheckIfBoundary(OMesh::EdgeHandle eh) {
		using namespace OpenMesh;

		HalfedgeHandle hh0 = mesh.halfedge_handle(eh, 0);
		HalfedgeHandle hh1 = mesh.halfedge_handle(eh, 1);

		return mesh.is_boundary(eh)
			|| mesh.is_boundary(mesh.to_vertex_handle(hh0))
			|| mesh.is_boundary(mesh.to_vertex_handle(hh1));
	}

	void DecimateRatio(float max_ratio) {
		StaticDecimator::DecimateCluster(mesh, max_ratio);
	}

	void CollapseEdges() {
		using namespace OpenMesh;

		mesh.request_edge_status();
		mesh.request_halfedge_status();
		mesh.request_vertex_status();
		mesh.request_face_status();

		int times = 100000;
		int face_count = mesh.n_faces();

		const auto compare_fun = [&](EdgeHandle _lhs, EdgeHandle _rhs) {
			return mesh.data(_lhs).error() < mesh.data(_rhs).error();
		};

		std::set <EdgeHandle, decltype(compare_fun)> edges_set(compare_fun);

		for (OMesh::EdgeIter edg_it(mesh.edges_begin()); edg_it != mesh.edges_end(); ++edg_it) {
			if (!CheckIfBoundary(*edg_it))
				edges_set.insert(*edg_it);
		}

		int deleted_count = 0;
		int generated_count = 0;

		while (edges_set.size() != 0 && generated_count < 9) {
			EdgeHandle to_be_deleted = *edges_set.begin();
			edges_set.erase(edges_set.begin());

			if (mesh.is_collapse_ok(mesh.halfedge_handle(to_be_deleted, 0))) {
				HalfedgeHandle hh = mesh.halfedge_handle(to_be_deleted, 0);
				VertexHandle   rem_vh = mesh.to_vertex_handle(hh);
				VertexHandle   del_vh = mesh.to_vertex_handle(mesh.opposite_halfedge_handle(hh));

				mesh.set_point(rem_vh, GetNewPointByQEF2OMesh(rem_vh, del_vh, 0.002f));
				mesh.collapse(hh);

				for (OMesh::VertexFaceIter vfi(mesh.vf_begin(rem_vh)); vfi.is_valid(); ++vfi) {
					mesh.set_normal(*vfi, mesh.calc_face_normal(*vfi));
				}

				for (OMesh::VertexEdgeIter vei(mesh.ve_begin(rem_vh)); vei.is_valid(); ++vei) {
					edges_set.erase(*vei);
					if (!CheckIfBoundary(*vei))
					{
						mesh.data(*vei).set_error(CalculateErrorForEdge(*vei));
						edges_set.insert(*vei);
					}
				}
				deleted_count += 2;

				if (deleted_count % (face_count / 10) == 0) {
					edges_set.clear();
					std::string file_name = "output/output";
					file_name += std::to_string(generated_count++);
					file_name += ".obj";
					SaveMesh(file_name);

					for (OMesh::EdgeIter edg_it(mesh.edges_begin()); edg_it != mesh.edges_end(); ++edg_it) {
						if (!CheckIfBoundary(*edg_it))
							edges_set.insert(*edg_it);
					}
					printf("\tGenerated file: %d\n", generated_count);
					printf("\tDeleted count: %d\n", deleted_count);
					printf("\tFace count: %d\n", mesh.n_faces());
				}
			}
		}
	}

	void CollapseEdgesMCA() {
		using namespace OpenMesh;

		mesh.request_edge_status();
		mesh.request_halfedge_status();
		mesh.request_vertex_status();
		mesh.request_face_status();

		int times = 100000;
		int face_count = mesh.n_faces();

		int deleted_count = 0;
		int generated_count = 0;
		int bad_cycles = 0;

		while (generated_count < 9 && bad_cycles < 10) {
			float best_error = FLT_MAX;
			int mesh_edges_count = mesh.n_edges();
			EdgeHandle to_be_deleted;

			for (int i = 0; i < 9; i++) {
				EdgeHandle e(rand() % mesh_edges_count);
				if (mesh.status(e).deleted()) {
					i--;
					continue;
				}
				if (mesh.data(e).error() < best_error) {
					to_be_deleted = e;
					best_error = mesh.data(e).error();
				}
			}

			if (mesh.is_collapse_ok(mesh.halfedge_handle(to_be_deleted, 0))) {
				HalfedgeHandle hh = mesh.halfedge_handle(to_be_deleted, 0);
				VertexHandle   rem_vh = mesh.to_vertex_handle(hh);
				VertexHandle   del_vh = mesh.to_vertex_handle(mesh.opposite_halfedge_handle(hh));

				mesh.set_point(rem_vh, GetNewPointByQEF2OMesh(rem_vh, del_vh, 0.002f));
				mesh.collapse(hh);

				for (OMesh::VertexFaceIter vfi(mesh.vf_begin(rem_vh)); vfi.is_valid(); ++vfi) {
					mesh.set_normal(*vfi, mesh.calc_face_normal(*vfi));
				}

				deleted_count += 2;

				if (deleted_count % (face_count / 10) == 0) {
					mesh.garbage_collection();
					std::string file_name = "output/output";
					file_name += std::to_string(generated_count++);
					file_name += ".obj";
					SaveMesh(file_name);
					printf("\tGenerated file: %d\n", generated_count);
					printf("\tDeleted count: %d\n", deleted_count);
					printf("\tFace count: %d\n", mesh.n_faces());
				}
				bad_cycles = 0;
			}
			else {
				bad_cycles++;
			}
		}
	}

	void SaveMesh(const std::string& path) {
		mesh.garbage_collection();
		try
		{
			OpenMesh::IO::Options ropt;
			//ropt += OpenMesh::IO::Options::VertexNormal;
			ropt += OpenMesh::IO::Options::VertexTexCoord;

			if (!OpenMesh::IO::write_mesh(mesh, path, ropt))
			{
				std::cerr << "Cannot write mesh to file 'output.obj'" << std::endl;
			}
		}
		catch (std::exception& x)
		{
			std::cerr << x.what() << std::endl;
		}
	}

	void _CollapseLoop(OMesh::HalfedgeHandle _hh)
	{
		using namespace OpenMesh;
		HalfedgeHandle  h0 = _hh;
		HalfedgeHandle  h1 = mesh.next_halfedge_handle(h0);

		HalfedgeHandle  o0 = mesh.opposite_halfedge_handle(h0);
		HalfedgeHandle  o1 = mesh.opposite_halfedge_handle(h1);

		VertexHandle    v0 = mesh.to_vertex_handle(h0);
		VertexHandle    v1 = mesh.to_vertex_handle(h1);

		FaceHandle      fh = mesh.face_handle(h0);
		FaceHandle      fo = mesh.face_handle(o0);



		// is it a loop ?
		assert((mesh.next_halfedge_handle(h1) == h0) && (h1 != o0));


		// halfedge -> halfedge
		mesh.set_next_halfedge_handle(h1, mesh.next_halfedge_handle(o0));
		mesh.set_next_halfedge_handle(mesh.prev_halfedge_handle(o0), h1);


		// halfedge -> face
		/*mesh.set_face_handle(h1, fo);*/


		// vertex -> halfedge
		/*mesh.set_halfedge_handle(v0, h1);  */mesh.adjust_outgoing_halfedge(v0);
		/*mesh.set_halfedge_handle(v1, o1);  */mesh.adjust_outgoing_halfedge(v1);


		// face -> halfedge
		//if (fo.is_valid() && mesh.halfedge_handle(fo) == o0)
		//{
		//	mesh.set_halfedge_handle(fo, h1);
		//}

		// delete stuff
		//if (fh.is_valid())
		//{
		//	mesh.set_halfedge_handle(fh, mesh.InvalidHalfedgeHandle);
		//	mesh.status(fh).set_deleted(true);
		//}
		//mesh.status(mesh.edge_handle(h0)).set_deleted(true);
		//if (mesh.has_halfedge_status())
		//{
		//	mesh.status(h0).set_deleted(true);
		//	mesh.status(o0).set_deleted(true);
		//}
	}
	void Prinfo(OMesh::HalfedgeHandle _hh) {
		std::cout << mesh.prev_halfedge_handle(mesh.prev_halfedge_handle(mesh.prev_halfedge_handle(_hh)))
			<< (mesh.status(mesh.prev_halfedge_handle(mesh.prev_halfedge_handle(mesh.prev_halfedge_handle(_hh)))).deleted() ? " d" : "")
			<< (mesh.is_boundary(mesh.prev_halfedge_handle(mesh.prev_halfedge_handle(mesh.prev_halfedge_handle(_hh)))) ? "b" : "")
			<< (mesh.is_boundary(mesh.opposite_halfedge_handle(mesh.prev_halfedge_handle(mesh.prev_halfedge_handle(mesh.prev_halfedge_handle(_hh))))) ? "b" : "")
			<< " -> " << mesh.prev_halfedge_handle(mesh.prev_halfedge_handle(_hh))
			<< (mesh.status(mesh.prev_halfedge_handle(mesh.prev_halfedge_handle(_hh))).deleted() ? " d" : "")
			<< (mesh.is_boundary(mesh.prev_halfedge_handle(mesh.prev_halfedge_handle(_hh))) ? "b" : "")
			<< (mesh.is_boundary(mesh.opposite_halfedge_handle(mesh.prev_halfedge_handle(mesh.prev_halfedge_handle(_hh)))) ? "b" : "")
			<< " -> " << mesh.prev_halfedge_handle(_hh)
			<< (mesh.status(mesh.prev_halfedge_handle(_hh)).deleted() ? " d" : "")
			<< (mesh.is_boundary(mesh.prev_halfedge_handle(_hh)) ? "b" : "")
			<< (mesh.is_boundary(mesh.opposite_halfedge_handle(mesh.prev_halfedge_handle(_hh))) ? "b" : "")
			<< " ->( " << _hh
			<< (mesh.status(_hh).deleted() ? " d" : "")
			<< (mesh.is_boundary(_hh) ? "b" : "")
			<< (mesh.is_boundary(mesh.opposite_halfedge_handle(_hh)) ? "b" : "")
			<< " )-> " << mesh.next_halfedge_handle(_hh)
			<< (mesh.status(mesh.next_halfedge_handle(_hh)).deleted() ? " d" : "")
			<< (mesh.is_boundary(mesh.next_halfedge_handle(_hh)) ? "b" : "")
			<< (mesh.is_boundary(mesh.opposite_halfedge_handle(mesh.next_halfedge_handle(_hh))) ? "b" : "")
			<< " -> " << mesh.next_halfedge_handle(mesh.next_halfedge_handle(_hh))
			<< (mesh.status(mesh.next_halfedge_handle(mesh.next_halfedge_handle(_hh))).deleted() ? " d" : "")
			<< (mesh.is_boundary(mesh.next_halfedge_handle(mesh.next_halfedge_handle(_hh))) ? "b" : "")
			<< (mesh.is_boundary(mesh.opposite_halfedge_handle(mesh.next_halfedge_handle(mesh.next_halfedge_handle(_hh)))) ? "b" : "")
			<< std::endl;
	}
	void CollapseEdgeWithFaceDelete(OMesh::HalfedgeHandle _edge) {
		using namespace OpenMesh;

		HalfedgeHandle h0 = _edge;
		HalfedgeHandle h0_prev = mesh.prev_halfedge_handle(_edge);
		HalfedgeHandle h0_next = mesh.next_halfedge_handle(_edge);

		HalfedgeHandle h1 = mesh.opposite_halfedge_handle(h0);
		HalfedgeHandle h1_prev = mesh.prev_halfedge_handle(h1);
		HalfedgeHandle h1_next = mesh.next_halfedge_handle(h1);

		VertexHandle   v0 = mesh.to_vertex_handle(h1);
		VertexHandle   v1 = mesh.to_vertex_handle(h0);

		FaceHandle     f0 = mesh.face_handle(h0);
		FaceHandle     f1 = mesh.face_handle(h1);

		if (mesh.status(v1).deleted() || mesh.status(v0).deleted()) return;

		// 1. Set new position for v1 TODO
		//mesh.set_point(v0, OMesh::Point(new_point.x, new_point.y, new_point.z));

		// 2. Remove f1 and f2
		mesh.status(f0).set_deleted(true);
		mesh.status(f1).set_deleted(true);

		Prinfo(h0);
		Prinfo(h1);

		std::cout << v0 << " " << v1 << std::endl;
		std::cout << "HEH: " << mesh.halfedge_handle(v1) << (mesh.status(mesh.halfedge_handle(v1)).deleted() ? "d" : "") << std::endl;
		std::cout << (mesh.status(v0).deleted() ? "d" : "") << " " << (mesh.status(v1).deleted() ? "d" : "") << std::endl;

		printf("\n");


		int counter = 0;
		// 4. Set all incoming and outgoing halfedges from v2 -> v1
		for (OMesh::VertexIHalfedgeIter vih_it(mesh.vih_iter(v1)); vih_it.is_valid(); ++vih_it) {
			std::cout << *vih_it << (mesh.status(*vih_it).deleted() ? "d" : "") << std::endl;
			mesh.set_vertex_handle(*vih_it, v0);
			if (++counter > 30)
				break;
		}

		mesh.set_next_halfedge_handle(h0_prev, h0_next);
		mesh.set_next_halfedge_handle(h1_prev, h1_next);

		mesh.set_prev_halfedge_handle(h0_next, h0_prev);
		mesh.set_prev_halfedge_handle(h1_next, h1_prev);

		//if (mesh.halfedge_handle(v0) == h0) 
		//	mesh.set_halfedge_handle(v0, h0_next);
		//mesh.adjust_outgoing_halfedge(v0);

		// 3. Remove e
		//mesh.delete_edge(mesh.edge_handle(h0));

		mesh.status(mesh.edge_handle(h0)).set_deleted(true);
		mesh.status(h0).set_deleted(true);
		mesh.status(h1).set_deleted(true);

		// 5. Set h2 face to f3 and h4 face to f4 
		HalfedgeHandle f0_nhe = mesh.next_halfedge_handle(h0);
		HalfedgeHandle f0_nnhe = mesh.next_halfedge_handle(f0_nhe);

		HalfedgeHandle f0_nheo = mesh.opposite_halfedge_handle(f0_nhe);
		HalfedgeHandle f0_nnheo = mesh.opposite_halfedge_handle(f0_nnhe);

		VertexHandle   f0_vh = mesh.to_vertex_handle(f0_nhe);

		FaceHandle     f3 = mesh.face_handle(f0_nheo);

		mesh.set_face_handle(f0_nnhe, f3);
		mesh.set_halfedge_handle(f3, f0_nnhe);

		mesh.set_next_halfedge_handle(f0_nnhe, mesh.next_halfedge_handle(f0_nheo));
		mesh.set_prev_halfedge_handle(f0_nnhe, mesh.prev_halfedge_handle(f0_nheo));
		mesh.set_next_halfedge_handle(mesh.prev_halfedge_handle(f0_nheo), f0_nnhe);

		// --------------------------------------------------------------------- //

		HalfedgeHandle f1_nhe = mesh.next_halfedge_handle(h1);
		HalfedgeHandle f1_nnhe = mesh.next_halfedge_handle(f1_nhe);

		HalfedgeHandle f1_nheo = mesh.opposite_halfedge_handle(f1_nhe);
		HalfedgeHandle f1_nnheo = mesh.opposite_halfedge_handle(f1_nnhe);

		VertexHandle   f1_vh = mesh.to_vertex_handle(f1_nhe);

		FaceHandle     f4 = mesh.face_handle(f1_nheo);


		mesh.set_face_handle(f1_nnhe, f4);
		mesh.set_halfedge_handle(f4, f1_nnhe);

		mesh.set_next_halfedge_handle(f1_nnhe, mesh.next_halfedge_handle(f1_nheo));
		mesh.set_prev_halfedge_handle(f1_nnhe, mesh.prev_halfedge_handle(f1_nheo));
		mesh.set_next_halfedge_handle(mesh.prev_halfedge_handle(f1_nheo), f1_nnhe);

		// 6. Remove v2

		mesh.status(v1).set_deleted(true);

		// 7. Remove edge between f1 and f3; f2 and f4
		//mesh.set_next_halfedge_handle(mesh.prev_halfedge_handle(f0_nheo), mesh.next_halfedge_handle(f0_nheo));
		//mesh.set_next_halfedge_handle(mesh.prev_halfedge_handle(f0_nhe), mesh.next_halfedge_handle(f0_nhe));

		//mesh.set_next_halfedge_handle(mesh.next_halfedge_handle(f0_nheo), mesh.prev_halfedge_handle(f0_nheo));
		//mesh.set_next_halfedge_handle(mesh.next_halfedge_handle(f0_nhe), mesh.prev_halfedge_handle(f0_nhe));

		//mesh.delete_edge(mesh.edge_handle(f0_nheo));

		mesh.status(f0_nheo).set_deleted(true);
		mesh.status(f0_nhe).set_deleted(true);
		mesh.status(mesh.edge_handle(f0_nheo)).set_deleted(true);

		// --------------------------------------------------------------------- //

		//mesh.set_next_halfedge_handle(mesh.prev_halfedge_handle(f1_nheo), mesh.next_halfedge_handle(f1_nheo));
		//mesh.set_next_halfedge_handle(mesh.prev_halfedge_handle(f1_nhe), mesh.next_halfedge_handle(f1_nhe));

		//mesh.set_next_halfedge_handle(mesh.next_halfedge_handle(f1_nheo), mesh.prev_halfedge_handle(f1_nheo));
		//mesh.set_next_halfedge_handle(mesh.next_halfedge_handle(f1_nhe), mesh.prev_halfedge_handle(f1_nhe));

		//if (mesh.halfedge_handle(v0) == f1_nhe)
		mesh.set_halfedge_handle(v0, f1_nnheo);
		mesh.set_halfedge_handle(f1_vh, f1_nnhe);
		mesh.set_halfedge_handle(f0_vh, f0_nnhe);


		//mesh.delete_edge(mesh.edge_handle(f1_nheo));
		mesh.status(f1_nheo).set_deleted(true);
		mesh.status(f1_nhe).set_deleted(true);
		mesh.status(mesh.edge_handle(f1_nheo)).set_deleted(true);

		//if (mesh.status(mesh.halfedge_handle(v0)).deleted()) {
		//	for (OMesh::VertexOHalfedgeIter vih_it(mesh.voh_iter(v0)); vih_it.is_valid(); ++vih_it) {
		//		if (!mesh.status(*vih_it).deleted()) {
		//			mesh.set_halfedge_handle(v0, *vih_it);
		//			break;
		//		}
		//	}
		//}

		//if (mesh.status(mesh.halfedge_handle(f1_vh)).deleted()) {
		//	for (OMesh::VertexOHalfedgeIter vih_it(mesh.voh_iter(f1_vh)); vih_it.is_valid(); ++vih_it) {
		//		if (!mesh.status(*vih_it).deleted()) {
		//			mesh.set_halfedge_handle(f1_vh, *vih_it);
		//			break;
		//		}
		//	}
		//}

		//if (mesh.status(mesh.halfedge_handle(f0_vh)).deleted()) {
		//	for (OMesh::VertexOHalfedgeIter vih_it(mesh.voh_iter(f0_vh)); vih_it.is_valid(); ++vih_it) {
		//		if (!mesh.status(*vih_it).deleted()) {
		//			mesh.set_halfedge_handle(f0_vh, *vih_it);
		//			break;
		//		}
		//	}
		//}
		//mesh.set_next_halfedge_handle(h0_next, mesh.next_halfedge_handle(mesh.next_halfedge_handle(h0_next)));
		//mesh.set_next_halfedge_handle(h1_next, mesh.next_halfedge_handle(mesh.next_halfedge_handle(h1_next)));
		//mesh.set_prev_halfedge_handle(h1_prev, f1_nnheo);

		//printf("\n");
		//Prinfo(h0_prev);
		//Prinfo(h1_prev);
		//Prinfo(f0_nnhe);
		//Prinfo(f1_nnhe);

		//if (mesh.next_halfedge_handle(mesh.next_halfedge_handle(h0_next)) == h0_next)
		//	_CollapseLoop(h0_next);
		//if (mesh.next_halfedge_handle(mesh.next_halfedge_handle(h1_next)) == h1_next)
		//	_CollapseLoop(h1_next);

		//if (mesh.next_halfedge_handle(mesh.next_halfedge_handle(f1_nnhe)) == f1_nnhe)
		//	_CollapseLoop(h1_next);
		//if (mesh.next_halfedge_handle(mesh.next_halfedge_handle(f0_nnhe)) == f0_nnhe)
		//	_CollapseLoop(h1_next);

		//mesh.adjust_outgoing_halfedge(v0);
		//mesh.adjust_outgoing_halfedge(f0_vh);
		//mesh.adjust_outgoing_halfedge(f1_vh);

		printf("Done.\n");
	}
	void collapse(OMesh::HalfedgeHandle _hh)
	{
		using namespace OpenMesh;
		HalfedgeHandle h0 = _hh;
		HalfedgeHandle h1 = mesh.next_halfedge_handle(h0);
		HalfedgeHandle o0 = mesh.opposite_halfedge_handle(h0);
		HalfedgeHandle o1 = mesh.next_halfedge_handle(o0);

		// remove edge
		collapse_edge(h0);

		// remove loops
		//if (h1 == mesh.next_halfedge_handle(mesh.next_halfedge_handle(h1)))
		//if (mesh.next_halfedge_handle(o1) == mesh.next_halfedge_handle(mesh.next_halfedge_handle(mesh.next_halfedge_handle(o1))))
		collapse_loop(mesh.next_halfedge_handle(o1));
		if (!mesh.status(h1).deleted()) {
			mesh.set_next_halfedge_handle(h1, mesh.prev_halfedge_handle(h1));
			collapse_loop(h1);
		}
	}
	void collapse_edge(OMesh::HalfedgeHandle _hh)
	{
		using namespace OpenMesh;
		HalfedgeHandle  h = _hh;
		HalfedgeHandle  hn = mesh.next_halfedge_handle(h);
		HalfedgeHandle  hp = mesh.prev_halfedge_handle(h);

		HalfedgeHandle  o = mesh.opposite_halfedge_handle(h);
		HalfedgeHandle  on = mesh.next_halfedge_handle(o);
		HalfedgeHandle  op = mesh.prev_halfedge_handle(o);

		FaceHandle      fh = mesh.face_handle(h);
		FaceHandle      fo = mesh.face_handle(o);

		VertexHandle    vh = mesh.to_vertex_handle(h);
		VertexHandle    vo = mesh.to_vertex_handle(o);



		// halfedge -> vertex
		for (OMesh::VertexIHalfedgeIter vih_it(mesh.vih_iter(vo)); vih_it.is_valid(); ++vih_it)
			mesh.set_vertex_handle(*vih_it, vh);

		if (mesh.status(hn).deleted())
			return;

		// halfedge -> halfedge
		mesh.set_next_halfedge_handle(hp, hn);
		mesh.set_next_halfedge_handle(op, on);
		mesh.set_prev_halfedge_handle(hn, hp);
		mesh.set_prev_halfedge_handle(op, on);

		if (hn != mesh.next_halfedge_handle(mesh.next_halfedge_handle(hn))) {
			Prinfo(hn);
			Prinfo(hn);
		}

		//// face -> halfedge
		//mesh.set_halfedge_handle(fh, hn);
		//mesh.set_halfedge_handle(fo, on);


		// vertex -> halfedge
		if (mesh.halfedge_handle(vh) == o)  mesh.set_halfedge_handle(vh, hn);
		mesh.adjust_outgoing_halfedge(vh);
		mesh.set_isolated(vo);

		// delete stuff
		mesh.status(mesh.edge_handle(h)).set_deleted(true);
		mesh.status(vo).set_deleted(true);
		mesh.status(h).set_deleted(true);
		mesh.status(o).set_deleted(true);
	}
	void collapse_loop(OMesh::HalfedgeHandle _hh)
	{
		using namespace OpenMesh;
		HalfedgeHandle  h0 = _hh;
		HalfedgeHandle  h1 = mesh.next_halfedge_handle(h0);

		HalfedgeHandle  o0 = mesh.opposite_halfedge_handle(h0);
		HalfedgeHandle  o1 = mesh.opposite_halfedge_handle(h1);

		VertexHandle    v0 = mesh.to_vertex_handle(h0);
		VertexHandle    v1 = mesh.to_vertex_handle(h1);

		FaceHandle      fh = mesh.face_handle(h0);
		FaceHandle      fo = mesh.face_handle(o0);

		if (h0 != mesh.next_halfedge_handle(mesh.next_halfedge_handle(h0)))
			Prinfo(h0);

		// halfedge -> halfedge
		mesh.set_next_halfedge_handle(h1, mesh.next_halfedge_handle(o0));
		mesh.set_next_halfedge_handle(mesh.prev_halfedge_handle(o0), h1);


		// halfedge -> face
		mesh.set_face_handle(h1, fo);


		// vertex -> halfedge
		mesh.set_halfedge_handle(v0, h1);  /*mesh.adjust_outgoing_halfedge(v0);*/
		mesh.set_halfedge_handle(v1, o1); /* mesh.adjust_outgoing_halfedge(v1);*/


		// face -> halfedge
		mesh.set_halfedge_handle(fo, h1);

		// Delete face
		//mesh.set_halfedge_handle(fh, OMesh::InvalidHalfedgeHandle);
		mesh.status(fh).set_deleted(true);

		// Delete edge
		mesh.status(mesh.edge_handle(h0)).set_deleted(true);
		mesh.status(h0).set_deleted(true);
		mesh.status(o0).set_deleted(true);
	}
};