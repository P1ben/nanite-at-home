#pragma once
#include <metis.h>
#include "OMesh.h"
#include "Nanite/Cluster.h"

class GraphPartitioner {
public:
	static std::vector<idx_t> PartitionMesh(const OMesh& mesh, uint32_t group_count) {
		idx_t faces_count = mesh.n_faces();
		//idx_t faces_count = 6;
		idx_t n_groups = group_count;
		//idx_t n_groups = 10;

		idx_t bal_constraints = 1;

		std::vector<idx_t> xadj;
		std::vector<idx_t> adjncy;

		xadj.reserve(faces_count);
		adjncy.reserve(3 * faces_count);

		idx_t curr_num = 0;
		xadj.push_back(curr_num);

		for (OMesh::FaceIter fi = mesh.faces_begin(); fi != mesh.faces_end(); ++fi) {
			for (OMesh::ConstFaceFaceIter ffi = mesh.cff_begin(*fi); ffi.is_valid(); ++ffi) {
				adjncy.push_back(ffi->idx());
				curr_num++;
			}
			xadj.push_back(curr_num);
		}

		std::vector<idx_t> output0(faces_count, 0);
		std::vector<idx_t> output1(3 * faces_count , 0);
		//output.reserve(faces_count);

		idx_t comm_val = 0;
		idx_t* outp = (idx_t*)malloc(faces_count * sizeof(idx_t));


		 //* = can't be NULL
		int result = METIS_PartGraphKway(
			&faces_count, // *Number of vertices
			&bal_constraints, // *Number of balancing constraints (min 1)
			xadj.data(), // *First row of adjacency structure (xadj)
			adjncy.data(), // *Second row of adjacency structure (adjncy)
			NULL, // Vertex weights
			NULL, // Vertex size
			NULL, // Edge weight
			&n_groups, // *Parts to partition the graph into
			NULL, // Weight numero 900
			NULL, // Load imbalance tolerance array
			NULL, // Options
			&comm_val, // *Out: Total communication value
			output0.data()  // *Out: parts
		);

		//// * = can't be NULL
		//int result = METIS_PartMeshNodal (
		//	&faces_count, //*Number of elements
		//	&curr_num, //*Number of nodes
		//	xadj.data(), //*First array (eptr)
		//	adjncy.data(), //*Second array (eind)
		//	NULL, //Weight array
		//	NULL, //Smthn smthn communication value
		//	&n_groups, //*Parts to partition the mesh into
		//	NULL, //Another kind of weigths?
		//	NULL, //Options
		//	&comm_val, //*Out: communication volume
		//	output0.data(), //*Out: Element partition vector
		//	output1.data()  //*Out: Node partition vector
		//);

		//std::vector<int> group_cs(n_groups, 0);

		//for (unsigned part_i = 0; part_i < output0.size(); part_i++) {
		//	group_cs[output0[part_i]]++;
		//}

		/*for (int i = 0; i < group_cs.size(); i++) {
			printf("Group %d count: %d\n", i, group_cs[i]);
		}*/

		if (result == METIS_OK) {
			return output0;
		}
		else {
			printf("Baj van :(");
			return output0;
		}
	}

	static std::vector<idx_t> PartitionClusterArray(std::vector<Cluster>& clusters, int start_index, uint32_t group_count) {
		idx_t cluster_count = clusters.size() - start_index;
		//idx_t faces_count = 6;
		if (group_count == 1) {
			return std::vector<idx_t>(cluster_count, 0);
		}

		idx_t n_groups = group_count;
		//idx_t n_groups = 10;

		idx_t bal_constraints = 1;

		std::vector<idx_t> xadj;
		std::vector<idx_t> adjncy;

		xadj.reserve(cluster_count);
		adjncy.reserve(5 * cluster_count);

		idx_t curr_num = 0;
		xadj.push_back(curr_num);

		for (int i = start_index; i < clusters.size(); i++) {
			auto& neighs = clusters[i].GetNeighbours();
			for (int j = 0; j < neighs.size(); j++) {
				adjncy.push_back(neighs[j] - start_index);
				curr_num++;
			}
			xadj.push_back(curr_num);
		}

		std::vector<idx_t> output0(cluster_count, 0);

		idx_t comm_val = 0;

		//* = can't be NULL
		int result = METIS_PartGraphKway(
			&cluster_count, // *Number of vertices
			&bal_constraints, // *Number of balancing constraints (min 1)
			xadj.data(), // *First row of adjacency structure (xadj)
			adjncy.data(), // *Second row of adjacency structure (adjncy)
			NULL, // Vertex weights
			NULL, // Vertex size
			NULL, // Edge weight
			&n_groups, // *Parts to partition the graph into
			NULL, // Weight numero 900
			NULL, // Load imbalance tolerance array
			NULL, // Options
			&comm_val, // *Out: Total communication value
			output0.data()  // *Out: parts
		);

		//std::vector<int> group_cs(n_groups, 0);

		//for (unsigned part_i = 0; part_i < output0.size(); part_i++) {
		//	group_cs[output0[part_i]]++;
		//}

		if (result == METIS_OK) {
			return output0;
		}
		else {
			printf("Baj van :(");
			return output0;
		}
	}

	static std::vector<idx_t> PartitionMeshTest(const OMesh& mesh, uint32_t group_size) {
		idx_t nVertices = 6;
		idx_t nEdges = 7;
		idx_t nWeights = 1;
		idx_t nParts = 2;

		idx_t objval;
		std::vector<idx_t> part(nVertices, 0);


		// Indexes of starting points in adjacent array
		std::vector<idx_t> xadj = { 0,2,5,7,9,12,14 };

		// Adjacent vertices in consecutive index order
		std::vector<idx_t> adjncy = { 1,3,0,4,2,1,5,0,4,3,1,5,4,2 };

		// Weights of vertices
		// if all weights are equal then can be set to NULL
		std::vector<idx_t> vwgt(nVertices * nWeights, 0);



		int ret = METIS_PartGraphKway(&nVertices, &nWeights, xadj.data(), adjncy.data(),
			NULL, NULL, NULL, &nParts, NULL,
			NULL, NULL, &objval, part.data());

		std::cout << ret << std::endl;

		for (unsigned part_i = 0; part_i < part.size(); part_i++) {
			std::cout << part_i << " " << part[part_i] << std::endl;
		}

		return part;
	}
};