#pragma once
#include "Cluster.h"
#include "../framework.h"
#include "../GraphPartitioner.h"
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>

#define BASE_CLUSTER_SIZE 128

class NaniteMesh : public Mesh {
	std::vector<Cluster> clusters;

	std::vector<CLUSTER_ID> active_clusters;

	// Current mesh
	std::vector<Vertex> vertices;
	std::vector<Face>   faces;

	struct LodTree {
		struct Node {
			CLUSTER_ID cluster_id;
			std::vector<Node> children;

			bool is_leaf = false;

			Node(CLUSTER_ID _cluster_id, bool _is_leaf) {
				cluster_id = _cluster_id;
				is_leaf = _is_leaf;
				children.reserve(4);
			}

			void AddChild(Cluster& cluster) {
				children.push_back(Node(cluster.GetId(), cluster.GetLeaf()));
			}

			Node* Find(CLUSTER_ID id) {
				if (cluster_id == id) {
					return this;
				}

				for (Node& chld : children) {
					Node* found = chld.Find(id);
					if (found) {
						return found;
					}
				}

				return nullptr;
			}

			void FillFaceVertRecursive(float cut_metric, Cluster& parent, std::vector<Cluster>& clusters, std::vector<Face>& faces, std::vector<Vertex>& verts) {
				if (clusters[cluster_id].ShouldShow(cut_metric, parent)) {
					std::vector<Face> clstr_fc = clusters[cluster_id].GetFacesWithOffset(verts.size());
					faces.insert(faces.end(), clstr_fc.begin(), clstr_fc.end());

					std::vector<Vertex> clstr_vtx = clusters[cluster_id].GetVertices();
					verts.insert(verts.end(), clstr_vtx.begin(), clstr_vtx.end());
				}
				else {
					for (auto& chld : children) {
						chld.FillFaceVertRecursive(cut_metric, clusters[cluster_id], clusters, faces, verts);
					}
				}
			}

			uint32_t CountSubtree() {
				uint32_t counter = 1;
				for (auto& chld : children) {
					counter += chld.CountSubtree();
				}

				return counter;
			}
		};

		std::vector<Node> root_nodes;

		uint32_t count = 0;

		LodTree() {
			root_nodes.reserve(2);
		}

		Node* FindNode(CLUSTER_ID id) {
			for (Node& root : root_nodes) {
				Node* result = root.Find(id);
				if (result) {
					return result;
				}
			}

			return nullptr;
		}

		void AddCluster(Cluster& cluster) {
			if (cluster.GetRoot()) {
				root_nodes.push_back(Node(cluster.GetId(), cluster.GetLeaf()));
			}
			else {
				auto& parents = cluster.GetParents();
				Node* parent = FindNode(parents[0]);
				if (parent) {
					parent->AddChild(cluster);
					count++;
				}
				else {
					printf("<Error> Couldn't insert cluster into tree\n");
					printf("<Error> Possible file corruption or wrong order\n");
				}
			}
		}

		void FillFaceVert(float cut_metric, std::vector<Cluster>& clusters, std::vector<Face>& faces, std::vector<Vertex>& verts) {
			faces.clear();
			verts.clear();

			for (auto& root : root_nodes) {
				root.FillFaceVertRecursive(cut_metric, clusters[root.cluster_id], clusters, faces, verts);
			}
		}

		uint32_t CountNodes() {
			uint32_t counter = 0;
			for (auto& root : root_nodes) {
				counter += root.CountSubtree();
			}

			return counter;
		}
	};

	LodTree lod_tree;

	float camera_last_dist = 0.f;


	int cluster_count = 0;
	int face_count = 0;

	void UpdateCurrentMesh() {
		this->faces.clear();
		this->vertices.clear();

		for (auto cluster_id : active_clusters) {
			std::vector<Face> clstr_fc = clusters[cluster_id].GetFacesWithOffset(vertices.size());
			this->faces.insert(this->faces.end(), clstr_fc.begin(), clstr_fc.end());

			std::vector<Vertex> clstr_vtx = clusters[cluster_id].GetVertices();
			this->vertices.insert(this->vertices.end(), clstr_vtx.begin(), clstr_vtx.end());
		}
	}

	void RecalculateFaceCount() {
		//face_count = 0;
		//for (auto cluster_id : active_clusters) {
		//	face_count += clusters[cluster_id].GetFaceCount();
		//}
		face_count = faces.size();
	}

	void UpdateDensityForClusters() {
		int counter = 0;
		for (auto& cluster : clusters) {
			if (!cluster.IsMarked()) {

				// Parental
				float area_sum = cluster.GetSurfaceArea();
				uint32_t triangle_count = cluster.GetFaceCount();
				for (auto par_sib : cluster.GetParentalSiblings()) {
					//clusters[par_sib].Mark();
					area_sum += clusters[par_sib].GetSurfaceArea();
					triangle_count += clusters[par_sib].GetFaceCount();
				}

				cluster.SetParentalArea(area_sum);
				//cluster.SetParentalDensity(triangle_count / area_sum);
				cluster.SetParentalDensity(triangle_count / area_sum);

				// Childsib
				area_sum = cluster.GetSurfaceArea();
				triangle_count = cluster.GetFaceCount();
				for (auto chld_sib : cluster.GetChildSiblings()) {
					//clusters[chld_sib].Mark();
					area_sum += clusters[chld_sib].GetSurfaceArea();
					triangle_count += clusters[chld_sib].GetFaceCount();
				}

				cluster.SetChildsibArea(area_sum);
				cluster.SetChildsibDensity(triangle_count / area_sum);
				//cluster.SetChildsibDensity(triangle_count / area_sum);

				cluster.PrintDensity();
				counter++;
			}
		}


		int ne_prob = 0;
		int parent_prob = 0;

		for (auto& cluster : clusters) {
			float cluster_ch_d = cluster.GetChildsibDensity();
			for (auto chld_sib : cluster.GetChildSiblings()) {
				if (clusters[chld_sib].GetChildsibDensity() != cluster_ch_d) {
					printf("\tProblem with cluster %d: \n", cluster.GetId());
					printf("\t\tSelf child sibling density:       %f\n", cluster_ch_d);
					printf("\t\tCluster %d child sibling density: %f\n", chld_sib, clusters[chld_sib].GetChildsibDensity());
					ne_prob++;
				}
			}
		}

		bool found_err = false;

		for (auto& cluster : clusters) {
			for (auto chld_par : cluster.GetParents()) {
				if (clusters[chld_par].GetChildsibDensity() >= cluster.GetParentalDensity()) {
					printf("\tParent problem with cluster %d: \n", cluster.GetId());
					printf("\t\tSelf child sibling density:       %f\n", cluster.GetParentalDensity());
					printf("\t\tParent cluster %d child sibling density: %f\n", chld_par, clusters[chld_par].GetChildsibDensity());
					printf("\t\tSelf area: %f\n", cluster.GetParentalArea());
					printf("\t\tParent cluster %d area: %f\n", chld_par, clusters[chld_par].GetChildsibArea());
					printf("\t\tCluster childsib count: %d\n", cluster.GetChildSiblings().size());

					cluster.PrintDetails();
					parent_prob++;
					found_err = false;
					//clusters[chld_par].Mark();
				}
			}
			//cluster.Mark();

			//for (auto chld_sib : cluster.GetChildSiblings()) {
			//	clusters[chld_sib].Mark();
			//}

			if (found_err)
				break;

		}

		printf("Clusters affected: %d\n", counter);
		printf("\tNot equal problems: %d\n", ne_prob);
		printf("\tParental problems: %d\n", parent_prob);
	}

public:
	NaniteMesh() {}
	NaniteMesh(OMesh& mesh) {
		uint32_t face_count = mesh.n_faces();
		uint32_t group_count = face_count / BASE_CLUSTER_SIZE + 1;

		clusters = std::vector<Cluster>(group_count);

		for (int i = 0; i < clusters.size(); i++) {
			clusters[i].SetId(cluster_count++);
			clusters[i].SetLeaf(true);
		}

		printf("-Partitioning mesh-\n");

		std::vector<idx_t> partitions = GraphPartitioner::PartitionMesh(mesh, group_count);

		int i = 0;
		for (OMesh::FaceIter fi = mesh.faces_begin(); fi != mesh.faces_end(); ++fi) {
			int partition_num = partitions[i++];
			clusters[partition_num].CopyTriangle(mesh, *fi);

			for (OMesh::FaceFaceIter fi_ffi(mesh.ff_begin(*fi)); fi_ffi.is_valid(); ++fi_ffi) {
				int id = fi_ffi->idx();
				if (partitions[id] != partition_num) {
					clusters[partition_num].AddNeighbour(partitions[id]);
				}
			}

			if (i % (face_count / 10) == 0) {
				printf(".");
			}
		}
		printf("DONE\n");

		for (auto& clstr : clusters) {
			face_count += clstr.GetFaceCount();
			clstr.CalculateCenterFromInnerMesh();
		}

		RecalculateFaceCount();

		faces.reserve(face_count);
		vertices.reserve(face_count * 1.5f);

		UpdateCurrentMesh();
		this->SetUpdated(true);
	}

	NaniteMesh(const std::string& folder_path) {
		int i = 0;
		std::vector<std::string> cluster_files;
		std::string config_file;

		for (const auto& entry : std::experimental::filesystem::directory_iterator(folder_path)) {
			//printf("%s\n", entry.path().string().c_str());
			std::string path = entry.path().string();
			if (str_has_suffix(path, ".conf")) {
				config_file = path;
			}
			else {
				cluster_files.push_back(path);
			}
			/*clusters.push_back(Cluster());
			clusters[clusters.size() - 1].Load(entry.path().string());
			clusters[clusters.size() - 1].SetId(i++);*/
		}

		std::sort(cluster_files.begin(), cluster_files.end());

		clusters = std::vector<Cluster>(cluster_files.size(), Cluster());

		// Loading cluster mesh data
		for (int i = 0; i < cluster_files.size(); i++) {
			clusters[i].Load(cluster_files[i]);
		}

		// Loading cluster config
		std::ifstream config(config_file);

		for (int i = 0; i < clusters.size(); i++) {
			std::string line;

			// Set Id
			std::getline(config, line);
			{
				CLUSTER_ID temp;
				std::istringstream iss(line);
				iss >> temp;
				clusters[i].SetId(temp);
			}

			// Set Neighbours
			std::getline(config, line);
			{
				CLUSTER_ID temp;
				std::istringstream iss(line);
				while (iss >> temp) {
					clusters[i].AddNeighbour(temp);
				}
			}

			// Set Parents
			std::getline(config, line);
			{
				CLUSTER_ID temp;
				std::istringstream iss(line);
				while (iss >> temp) {
					clusters[i].AddParent(temp);
				}
			}

			// Set Parental Siblings
			std::getline(config, line);
			{
				CLUSTER_ID temp;
				std::istringstream iss(line);
				while (iss >> temp) {
					clusters[i].AddParentalSibling(temp);
				}
			}

			// Set Children
			std::getline(config, line);
			{
				CLUSTER_ID temp;
				std::istringstream iss(line);
				while (iss >> temp) {
					clusters[i].AddChild(temp);
				}
			}

			// Set Child Siblings
			std::getline(config, line);
			{
				CLUSTER_ID temp;
				std::istringstream iss(line);
				while (iss >> temp) {
					clusters[i].AddChildSibling(temp);
				}
			}

			// Set IsLeaf
			std::getline(config, line);
			{
				CLUSTER_ID temp;
				std::istringstream iss(line);
				iss >> temp;
				clusters[i].SetLeaf(temp ? true : false);
			}

			// Set IsRoot
			std::getline(config, line);
			{
				CLUSTER_ID temp;
				std::istringstream iss(line);
				iss >> temp;
				clusters[i].SetRoot(temp ? true : false);
			}

			// Set Center
			std::getline(config, line);
			{
				vec3 temp;
				std::istringstream iss(line);
				iss >> temp.x >> temp.y >> temp.z;
				clusters[i].SetCenter(temp);
			}

			// Set error value
			std::getline(config, line);
			{
				float temp;
				std::istringstream iss(line);
				iss >> temp;
				clusters[i].SetError(temp);
			}
		}

		config.close();

		for (auto it = clusters.rbegin(); it != clusters.rend(); ++it)
		{
			lod_tree.AddCluster(*it);
		}

		printf("\tLodTree size: %u, Clusters list size: %u\n", lod_tree.CountNodes(), clusters.size());

		for (Cluster& clr : clusters) {
			if (clr.GetLeaf()) {
				active_clusters.push_back(clr.GetId());
			}
			clr.FixBoundaryNormals2(clusters);
		}

		RecalculateFaceCount();
		UpdateDensityForClusters();

		faces.reserve(face_count);
		vertices.reserve(face_count);

		UpdateCurrentMesh();
		this->SetUpdated(true);
	}

	NaniteMesh(const std::vector<Cluster>& clstr) {
		clusters.reserve(clstr.size());

		for (auto& cl : clstr) {
			clusters.push_back(cl);
		}

		RecalculateFaceCount();

		faces.reserve(face_count);
		vertices.reserve(face_count * 1.5f);

		UpdateCurrentMesh();
		this->SetUpdated(true);
	}

	void DecreaseQuality() {
		std::vector<CLUSTER_ID> activate;
		printf("Active clusters len before: %d\n", active_clusters.size());

		for (int i = 0; i < active_clusters.size(); i++) {
			Cluster& cluster = clusters[active_clusters[i]];
			if (!cluster.GetRoot()) {
				auto& parents = cluster.GetParents();
				activate.insert(activate.end(), parents.begin(), parents.end());
				
				auto& parental_sibl = cluster.GetParentalSiblings();
				for (CLUSTER_ID id : parental_sibl) {
					auto position = std::find(active_clusters.begin(), active_clusters.end(), id);
					if (position != active_clusters.end()) {
						active_clusters.erase(position);
					}
				}

				active_clusters.erase(active_clusters.begin() + i);
				i--;
			}
		}

		printf("Activate len: %d\n", activate.size());
		printf("Active clusters len after: %d\n", active_clusters.size());

		active_clusters.insert(active_clusters.end(), activate.begin(), activate.end());

		UpdateCurrentMesh();
		RecalculateFaceCount();
		SetUpdated(true);
	}

	void IncreaseQuality() {
		std::vector<CLUSTER_ID> activate;
		printf("Active clusters len before: %d\n", active_clusters.size());

		for (int i = 0; i < active_clusters.size(); i++) {
			Cluster& cluster = clusters[active_clusters[i]];
			if (!cluster.GetLeaf()) {
				auto& parents = cluster.GetChildren();
				activate.insert(activate.end(), parents.begin(), parents.end());

				auto& parental_sibl = cluster.GetChildSiblings();
				for (CLUSTER_ID id : parental_sibl) {
					auto position = std::find(active_clusters.begin(), active_clusters.end(), id);
					if (position != active_clusters.end()) {
						active_clusters.erase(position);
					}
				}

				active_clusters.erase(active_clusters.begin() + i);
				i--;
			}
		}

		printf("Activate len: %d\n", activate.size());
		printf("Active clusters len after: %d\n", active_clusters.size());

		active_clusters.insert(active_clusters.end(), activate.begin(), activate.end());

		UpdateCurrentMesh();
		RecalculateFaceCount();
		SetUpdated(true);
	}

	void _SetParentStepRecursive(Cluster& source, float step) {
		/*if (source.GetUpperBoundary() > 0.0f)
			printf("Plswork: %f\n", source.GetUpperBoundary());*/

		//if (source.GetLeaf()) {
		//	printf("Plswork: %f\n", source.GetUpperBoundary());
		//}
		for (auto& parent : source.GetParents()) {
			clusters[parent].SetLowerBoundary(source.GetUpperBoundary());
			if (!clusters[parent].GetRoot()) {
				clusters[parent].SetUpperBoundary(clusters[parent].GetLowerBoundary() + step);
				_SetParentStepRecursive(clusters[parent], step);
			}
		}
	}

	void SetChangeStepForClusters(float step) {
		for (auto clstr : active_clusters) {
			if (clusters[clstr].GetLeaf()) {
				clusters[clstr].SetUpperBoundary(step);
				//printf("%f\n", step);
				//printf("Plswork: %f\n", clusters[clstr].GetUpperBoundary());
				_SetParentStepRecursive(clusters[clstr], step);
			}
		}
	}

	void Update(float center_distance_from_camera) override {
		lod_tree.FillFaceVert(center_distance_from_camera, clusters, faces, vertices);
		RecalculateFaceCount();
		SetUpdated(true);
		return;

		std::vector<CLUSTER_ID> activate;
		int change_count = 0;

		for (int i = 0; i < active_clusters.size(); i++) {
			Cluster& cluster = clusters[active_clusters[i]];

			if (cluster.IsMarked()) {
				continue;
			}

			Cluster::LodDecision decision = cluster.IsUpdateRequired(center_distance_from_camera);

			switch (decision) {
				case Cluster::LodDecision::DECREASE_QUALITY:
				{
					/*printf("DECREASE_QUALITY\n");
					auto& parents = cluster.GetParents();

					activate.insert(activate.end(), parents.begin(), parents.end());

					auto& parental_sibl = cluster.GetParentalSiblings();
					for (CLUSTER_ID id : parental_sibl) {
						auto position = std::find(active_clusters.begin(), active_clusters.end(), id);
						if (position != active_clusters.end()) {
							active_clusters.erase(position);
						}
					}*/
					
					auto& parents = cluster.GetParents();
					//if (parents.size() != 2)
					//	printf("Interesting");

					activate.insert(activate.end(), parents.begin(), parents.end());

					for (auto par_sib : cluster.GetParentalSiblings()) {
						clusters[par_sib].Mark();
						//if (std::find(active_clusters.begin(), active_clusters.end(), par_sib) == active_clusters.end()) {
						//	printf("Not found");
						//}
					}

					cluster.Mark();
					change_count++;
					break;
				}
				case Cluster::LodDecision::INCREASE_QUALITY: 
				{
					/*printf("INCREASE_QUALITY\n");
					auto& parents = cluster.GetChildren();
					activate.insert(activate.end(), parents.begin(), parents.end());

					auto& parental_sibl = cluster.GetChildSiblings();
					for (CLUSTER_ID id : parental_sibl) {
						auto position = std::find(active_clusters.begin(), active_clusters.end(), id);
						if (position != active_clusters.end()) {
							active_clusters.erase(position);
						}
					}*/

					auto& parents = cluster.GetChildren();
					activate.insert(activate.end(), parents.begin(), parents.end());

					for (auto child_sib : cluster.GetChildSiblings()) {
						clusters[child_sib].Mark();
						//if (std::find(active_clusters.begin(), active_clusters.end(), child_sib) == active_clusters.end()) {
						//	printf("Not found");
						//}
					}

					cluster.Mark();
					change_count++;
					break;
				}
				default:
					activate.push_back(cluster.GetId());
					cluster.Mark();
					//printf("DO_NOTHING\n");
			}
		}

		for (auto& cluster : clusters) {
			cluster.Unmark();
		}
		if (change_count > 0) {

			active_clusters = activate;

			UpdateCurrentMesh();
			RecalculateFaceCount();
			SetUpdated(true);
		}
	}

	void SimplifyClusters() {
		printf("-Simplifying Nanite Mesh-\n");
		int counter = 0;
		int clusters_size = clusters.size();
		for (auto& cl : clusters) {
			cl.Simplify();
			counter++;
			if (counter % (clusters_size / 10) == 0)
				printf(".");
		}
		printf("DONE\n");
	}

	NaniteMesh* GroupClusters(int group_size) {
		uint32_t group_count = clusters.size() / group_size + 1;

		printf("-Grouping clusters-\n");

		std::vector<Cluster> clstrs = std::vector<Cluster>(group_count);
		std::vector<idx_t> partitions = GraphPartitioner::PartitionClusterArray(clusters, 0, group_count);

		for (int i = 0; i < partitions.size(); i++) {
			clstrs[partitions[i]].Append(clusters[i]);
			if (i % ((partitions.size() / 10) + 1) == 0) {
				printf(".");
			}
		}
		printf("DONE\n");

		return new NaniteMesh(clstrs);
	}

	std::vector<Cluster> GroupClusters(int start_index, int group_size) {
		uint32_t group_count;
		if (clusters.size() - start_index > 4) {
			group_count = (clusters.size() - start_index) / group_size + 1;
		}
		else {
			group_count = 1;
		}

		printf("-Grouping clusters-\n");

		std::vector<Cluster> clstrs = std::vector<Cluster>(group_count);

		for (int i = 0; i < clstrs.size(); i++) {
			clstrs[i].SetId(i);
		}

		std::vector<idx_t> partitions = GraphPartitioner::PartitionClusterArray(clusters, start_index, group_count);

		for (int i = 0; i < partitions.size(); i++) {
			clstrs[partitions[i]].Append(clusters[i + start_index]);
			clstrs[partitions[i]].AddChild(clusters[i + start_index].GetId());
			clusters[i + start_index].AddParent(clstrs[partitions[i]].GetId());
			if (i % ((partitions.size() / 10) + 1) == 0) {
				printf(".");
			}
		}
		printf("DONE\n");

		return clstrs;
	}

	// Only to be used on the result of GroupClusters(int, int)
	void FixNeighbours(std::vector<Cluster>& cluster_list) {
		printf("-Fixing Neighbours-\n");
		int cluster_list_size = cluster_list.size();
		int i = 0;
		for (Cluster& cl : cluster_list) {
			CLUSTER_ID id = cl.GetId();
			for (CLUSTER_ID chld : cl.GetChildren()) {
				for (CLUSTER_ID chld_neig : clusters[chld].GetNeighbours()) {
					auto& parents = clusters[chld_neig].GetParents();
					if (parents.size() > 0 && parents[0] != id) {
						cl.AddNeighbour(parents[0]);
					}
				}
			}
			i++;
			if (i % ((cluster_list_size / 10) + 1) == 0) {
				printf(".");
			}
		}
		printf("DONE\n");
	}

	void FinalizeClusters(std::vector<Cluster>& cluster_list) {
		int cluster_list_size = cluster_list.size();
		for (auto& clstr : cluster_list) {
			// Creating new clusters
			clusters.push_back(Cluster());
			clusters.push_back(Cluster());

			// Getting ids of new clusters
			CLUSTER_ID cli_a = clusters.size() - 2;
			CLUSTER_ID cli_b = clusters.size() - 1;

			// Setting Id for clusters
			clusters[cli_a].SetId(cluster_count++);
			clusters[cli_b].SetId(cluster_count++);

			// Setting error for new clusters
			clusters[cli_a].SetError(clstr.GetError());
			clusters[cli_b].SetError(clstr.GetError());

			OMesh& cl_mesh = clstr.GetInnerMesh();

			// Dividing cluster to 2 parts
			auto parts = GraphPartitioner::PartitionMesh(cl_mesh, 2);

			printf("-Finalize Clusters Separation Part-\n");

			// Sorting triangles
			int i = 0;
			for (OMesh::FaceIter fi = cl_mesh.faces_begin(); fi != cl_mesh.faces_end(); ++fi) {
				int partition_num = parts[i];
				if (parts[i++] == 1) {
					clusters[cli_a].CopyTriangle(cl_mesh, *fi);
				}
				else {
					clusters[cli_b].CopyTriangle(cl_mesh, *fi);
				}

				if (i % ((parts.size() / 10) + 1) == 0) {
					printf(".");
				}
			}
			printf("DONE\n");

			// Setting up children-parent connection
			for (auto chldrn : clstr.GetChildren()) {
				clusters[chldrn].EmptyParents();
				clusters[chldrn].AddParent(cli_a);
				clusters[chldrn].AddParent(cli_b);

				for (auto chldrn_2 : clstr.GetChildren()) {
					if (chldrn != chldrn_2) {
						clusters[chldrn].AddParentalSibling(chldrn_2);
					}
				}

				clusters[cli_a].AddChild(chldrn);
				clusters[cli_b].AddChild(chldrn);
			}

			clusters[cli_a].AddChildSibling(cli_b);
			clusters[cli_b].AddChildSibling(cli_a);

			//Fixing neighbours (very slow and bad)
			for (auto neig : clstr.GetNeighbours()) {
				if (neig < cluster_list.size()) {
					cluster_list[neig].RemoveNeighbour(clstr.GetId());
				}
				else {
					clusters[neig].RemoveNeighbour(clstr.GetId());
				}
				clusters[cli_a].AddNeighbour(neig);
				clusters[cli_b].AddNeighbour(neig);
			}

			for (auto neig : clstr.GetNeighbours()) {
				if (neig < cluster_list.size()) {
					if (clusters[cli_a].IsNeighbour(cluster_list[neig])) {
						cluster_list[neig].AddNeighbour(cli_a);
					}
					else {
						clusters[cli_a].RemoveNeighbour(neig);
					}

					if (clusters[cli_b].IsNeighbour(cluster_list[neig])) {
						cluster_list[neig].AddNeighbour(cli_b);
					}
					else {
						clusters[cli_b].RemoveNeighbour(neig);
					}
				}
				else {
					if (clusters[cli_a].IsNeighbour(clusters[neig])) {
						clusters[neig].AddNeighbour(cli_a);
					}
					else {
						clusters[cli_a].RemoveNeighbour(neig);
					}

					if (clusters[cli_b].IsNeighbour(clusters[neig])) {
						clusters[neig].AddNeighbour(cli_b);
					}
					else {
						clusters[cli_b].RemoveNeighbour(neig);
					}
				}
			}

			clusters[cli_a].CalculateCenterFromInnerMesh();
			clusters[cli_b].CalculateCenterFromInnerMesh();
		}
	}

	int CreateNewLayer(int prev_layer_start_index) {
		int prev_start = prev_layer_start_index;

		printf("-Creating New Layer-\n");

		std::vector<Cluster> temp = GroupClusters(prev_start, 4);
		FixNeighbours(temp);

		printf("-Simplifying clusters-\n");

		int i = 0;
		for (auto& clstr : temp) {
			clstr.Simplify();
			if (i++ % ((temp.size() / 10) + 1) == 0) {
				printf(".");
			}
		}
		printf("DONE\n");

		FinalizeClusters(temp);

		return temp.size();
	}

	void Generate() {
		int start_index = 0;
		int retval = 0;

		do {
			int curr_cluster_size = clusters.size();
			retval = CreateNewLayer(start_index);
			start_index = curr_cluster_size;
		}
		while (retval > 1);

		for (int i = 1; i <= 2; i++) {
			clusters[clusters.size() - i].SetRoot(true);
		}

		for (auto& clstr : clusters) {
			if (!clstr.GetRoot()) {
				vec3 sum = clstr.GetCenter();
				int counter = 1;

				for (CLUSTER_ID par_sib : clstr.GetParentalSiblings()) {
					sum += clusters[par_sib].GetCenter();
					counter++;
				}

				sum = sum / counter;
				clstr.SetCenter(sum);
			}
		}
	}

	void AddCluster(Cluster& clstr) {
		this->clusters.push_back(clstr);
	}

	void PrintClusterDetails() {
		for (int i = 0; i < clusters.size(); i++) {
			clusters[i].PrintDetails();
		}
	}

	void WriteClusterDetailsIntoFile(const std::string& file_name) {
		FILE* file = fopen(file_name.c_str(), "w");
		if (!file) {
			printf("Couldn't write into file\n");
			return;
		}

		for (int i = 0; i < clusters.size(); i++) {
			clusters[i].WriteDetailsIntoFile(file);
		}
		fclose(file);
	}

	const virtual std::vector<Vertex>& GetVertices() override {
		return this->vertices;
	}

	const virtual std::vector<Face>& GetFaces() override {
		return this->faces;
	}

	virtual int GetFaceCount() override {
		return face_count;
	}

	void Save(const std::string& folder_name) {
		std::string config_path = folder_name + '\\' + "config.conf";

		FILE* config_file = fopen(config_path.c_str(), "w");

		if (!config_file) {
			printf("Couldn't write nanite mesh into file");
			return;
		}

		int numbering_len = std::to_string(clusters.size()).length();

		for (int i = 0; i < clusters.size(); i++)
		{
			std::stringstream ss;
			ss << "cluster" << std::setfill('0') << std::setw(numbering_len) << i << ".obj";

			std::string file_name = folder_name + "\\" + ss.str();
			clusters[i].Save(file_name, config_file);
		}

		fclose(config_file);
	}
};