#pragma once
#include "../OMesh.h"
#include "../framework.h"
#include "../StaticDecimator.h"
#include "../StaticMesh.h"

class Cluster {
	CLUSTER_ID id = 0;
	CLUSTER_ID parent = ID_INVALID;

	OMesh inner_mesh;

	StaticMesh inner_static_mesh;

	std::vector<CLUSTER_ID> neighbours;

	std::vector<CLUSTER_ID> _parents;
	std::vector<CLUSTER_ID> _parental_siblings;
	std::vector<CLUSTER_ID> _children;
	std::vector<CLUSTER_ID> _child_siblings;

	bool marked = false;
	bool is_leaf = false;
	bool is_root = false;

	vec3 center;

	float lower_distance_boundary = 0.0f;
	float upper_distance_boundary = 0.0f;

	float parental_area = 0.0f;
	float parental_triangle_density = 0.0f;

	float childsib_area = 0.0f;
	float childsib_triangle_density = 0.0f;

	float camera_last_dist = 0.f;

	vec3 color;



	// Copies vertex into inner mesh, checking for duplicate entries
	// Returns the handle of the added vertex
	OMesh::VertexHandle CopyVertexNoDupl(const OMesh& mesh, OMesh::VertexHandle vh) {
		for (OMesh::VertexIter v_it = inner_mesh.vertices_begin(); v_it != inner_mesh.vertices_end(); ++v_it) {
			if (inner_mesh.point(*v_it) == mesh.point(vh))
				return *v_it;
		}

		OMesh::Point new_vert = mesh.point(vh);
		OMesh::Normal new_normal = mesh.normal(vh);

		OMesh::VertexHandle vh_n = inner_mesh.add_vertex(new_vert);
		inner_mesh.set_normal(vh_n, new_normal);

		return vh_n;
	}

public:

	enum LodDecision {
		DECREASE_QUALITY,
		DO_NOTHING,
		INCREASE_QUALITY
	};

	Cluster() {
		color.x = (float)rand() / RAND_MAX;
		color.y = (float)rand() / RAND_MAX;
		color.z = (float)rand() / RAND_MAX;
	}

	// Adds necessary vertices and the triangle provided to the cluster
	void CopyTriangle(const OMesh& mesh, OMesh::FaceHandle fh) {
		std::vector<OMesh::VertexHandle> temp;
		temp.reserve(3);

		for (OMesh::ConstFaceVertexIter fv_it(mesh.cfv_iter(fh)); fv_it.is_valid(); ++fv_it) {
			temp.push_back(CopyVertexNoDupl(mesh, *fv_it));
		}

		inner_mesh.set_normal(inner_mesh.add_face(temp[0], temp[1], temp[2]), mesh.normal(fh));
	}

	void Append(Cluster& cluster) {
		for (OMesh::FaceIter fi_other(cluster.inner_mesh.faces_begin()); fi_other != cluster.inner_mesh.faces_end(); ++fi_other) {
			this->CopyTriangle(cluster.inner_mesh, *fi_other);
		}
	}

	void Simplify() {
		//StaticDecimator::DecimateCluster(inner_mesh, 0.5f);
		StaticDecimator::DecimateClusterUntil(inner_mesh, 256);
	}

	const virtual std::vector<Vertex> GetVertices() {
		//std::vector<Vertex> retval;

		//for (OMesh::VertexIter v_it(inner_mesh.vertices_begin()); v_it != inner_mesh.vertices_end(); ++v_it) {
		//	auto& pos = inner_mesh.point(*v_it);
		//	auto& norm = inner_mesh.normal(*v_it);
		//	//printf("%f, %f, %f\n", norm[0], norm[1], norm[2]);

		//	//printf("Vertex_id: %d\n", v_it->idx());

		//	retval.push_back(
		//		Vertex(
		//			vec3(pos[0], pos[1], pos[2]),
		//			vec3(norm[0], norm[1], norm[2]),
		//			color
		//		)
		//	);
		//}

		//return retval;

		return inner_static_mesh.GetVertices();
	}

	bool ShouldShow(float metric, Cluster& parent) {
		//if (marked)
		//	return true;
		//else
		//	return false;

		if (is_leaf) {
			return true;
		}

		float calc_metric = 1 / metric * 1080;

		if (id > 1550) {
			printf("\tCluster %d: Metric: %f Current: %f\n", id, calc_metric, childsib_triangle_density);
		}
		return childsib_triangle_density > calc_metric;
	}

	LodDecision IsUpdateRequired(float center_distance_from_camera) {
		float distance = center_distance_from_camera;  //sqrt(center_distance_from_camera * center_distance_from_camera + length(center) * length(center));

		//printf("Cluster distance: %f\n", distance);

		//printf("Lower dist bound: %f\n", lower_distance_boundary);
		//printf("Upper dist bound: %f\n", upper_distance_boundary);

		float density_based_on_distance = distance / 100.f;
		float step_const = 1.2f;

		if (is_root) {
			printf("\tCluster %d: Metric: %f Current: %f\n", id, density_based_on_distance, childsib_triangle_density);
		}

		if (!is_leaf && camera_last_dist > distance && density_based_on_distance <= childsib_triangle_density) {
			camera_last_dist = distance;
			return INCREASE_QUALITY;
		}

		if (!is_root && camera_last_dist < distance && density_based_on_distance > parental_triangle_density) {
			camera_last_dist = distance;
			return DECREASE_QUALITY;
		}

		//if (!is_root && distance > upper_distance_boundary)
		//	return DECREASE_QUALITY;
		//if (!is_leaf && distance < lower_distance_boundary)
		//	return INCREASE_QUALITY;
		camera_last_dist = distance;
		return DO_NOTHING;
	}

	const virtual std::vector<Face> GetFacesWithOffset(int offset_amount) {
		/*std::vector<Face> retval;

		for (OMesh::FaceIter f_it(inner_mesh.faces_begin()); f_it != inner_mesh.faces_end(); ++f_it) {
			OMesh::HalfedgeHandle hh = inner_mesh.halfedge_handle(*f_it);

			Face f;
			f.a = inner_mesh.to_vertex_handle(hh).idx() + offset_amount;

			hh = inner_mesh.next_halfedge_handle(hh);
			f.b = inner_mesh.to_vertex_handle(hh).idx() + offset_amount;

			hh = inner_mesh.next_halfedge_handle(hh);
			f.c = inner_mesh.to_vertex_handle(hh).idx() + offset_amount;

			retval.push_back(f);
		}

		return retval;*/

		std::vector<Face> faces = inner_static_mesh.GetFaces();

		for (auto& face : faces) {
			face.a += offset_amount;
			face.b += offset_amount;
			face.c += offset_amount;
		}

		return faces;
	}

	void PrintDensity() {
		printf("Cluster %d\n", id);
		printf("\tParental dens: %f\n", parental_triangle_density);
		printf("\tChildsib dens: %f\n", childsib_triangle_density);
	}

	void CalculateCenterFromInnerMesh() {
		vec3 sum(0.0f, 0.0f, 0.0f);
		int counter = 0;

		for (OMesh::VertexIter vi(inner_mesh.vertices_begin()); vi != inner_mesh.vertices_end(); ++vi) {
			auto& point = inner_mesh.point(*vi);
			sum += vec3(point[0], point[1], point[2]);
			counter++;
		}

		center = sum / counter;
	}

	bool IsNeighbour(Cluster& neig) {
		for (OMesh::VertexIter vi_o(inner_mesh.vertices_begin()); vi_o != inner_mesh.vertices_end(); ++vi_o) {
			if (inner_mesh.is_boundary(*vi_o)) {
				for (OMesh::VertexIter vi_i(neig.inner_mesh.vertices_begin()); vi_i != neig.inner_mesh.vertices_end(); ++vi_i) {
					if (neig.inner_mesh.is_boundary(*vi_i)) {
						if (inner_mesh.point(*vi_o) == neig.inner_mesh.point(*vi_i)) {
							return true;
						}
					}
				}
			}
		}
		return false;
	}

	void AddNeighbour(CLUSTER_ID cluster) {
		if (std::find(neighbours.begin(), neighbours.end(), cluster) == neighbours.end()) {
			neighbours.push_back(cluster);
		}
	}

	void RemoveNeighbour(CLUSTER_ID cluster) {
		std::vector<CLUSTER_ID>::iterator position = std::find(neighbours.begin(), neighbours.end(), cluster);
		if (position != neighbours.end())
			neighbours.erase(position);
	}

	std::vector<CLUSTER_ID>& GetNeighbours() {
		return neighbours;
	}

	void SetId(CLUSTER_ID id) {
		this->id = id;
	}

	CLUSTER_ID GetId() {
		return id;
	}

	OMesh& GetInnerMesh() {
		return inner_mesh;
	}

	void SetLeaf(bool _is_leaf) {
		is_leaf = _is_leaf;
	}

	bool GetLeaf() {
		return is_leaf;
	}

	void SetRoot(bool _is_root) {
		is_root = _is_root;
	}

	bool GetRoot() {
		return is_root;
	}

	float GetLowerBoundary() {
		return lower_distance_boundary;
	}

	void SetLowerBoundary(float boundary) {
		lower_distance_boundary = boundary;
	}

	float GetUpperBoundary() {
		return upper_distance_boundary;
	}

	void SetUpperBoundary(float boundary) {
		upper_distance_boundary = boundary;
	}

	void AddParent(CLUSTER_ID parent_id) {
		if (std::find(_parents.begin(), _parents.end(), parent_id) == _parents.end())
			_parents.push_back(parent_id);
	}

	void EmptyParents() {
		_parents.clear();
	}

	std::vector<CLUSTER_ID>& GetParents() {
		return _parents;
	}

	void AddParentalSibling(CLUSTER_ID parental_sibling_id) {
		if (std::find(_parental_siblings.begin(), _parental_siblings.end(), parental_sibling_id) == _parental_siblings.end())
			_parental_siblings.push_back(parental_sibling_id);
	}

	std::vector<CLUSTER_ID>& GetParentalSiblings() {
		return _parental_siblings;
	}

	void AddChild(CLUSTER_ID child_id) {
		if (std::find(_children.begin(), _children.end(), child_id) == _children.end())
			_children.push_back(child_id);
	}

	std::vector<CLUSTER_ID>& GetChildren() {
		return _children;
	}

	void AddChildSibling(CLUSTER_ID child_sibling_id) {
		if (std::find(_child_siblings.begin(), _child_siblings.end(), child_sibling_id) == _child_siblings.end())
			_child_siblings.push_back(child_sibling_id);
	}

	std::vector<CLUSTER_ID>& GetChildSiblings() {
		return _child_siblings;
	}

	uint32_t GetFaceCount() {
		//return inner_mesh.n_faces();
		return inner_static_mesh.GetFaceCount();
	}

	vec3& GetCenter() {
		return center;
	}

	void SetCenter(vec3& _center) {
		center = _center;
	}

	void PrintDetails() {
		printf("\tCluster %d:\n", id);
		printf("\t\tIs Leaf: %s:\n", (is_leaf ? "true" : "false"));
		printf("\t\tNeighbours:\n");
		printf("\t\t\t");
		for (int i = 0; i < neighbours.size(); i++) {
			printf("%u ", neighbours[i]);
		}
		printf("\n");

		printf("\t\tParents:\n");
		printf("\t\t\t");
		for (int i = 0; i < _parents.size(); i++) {
			printf("%u ", _parents[i]);
		}
		printf("\n");

		printf("\t\tParental Siblings:\n");
		printf("\t\t\t");
		for (int i = 0; i < _parental_siblings.size(); i++) {
			printf("%u ", _parental_siblings[i]);
		}
		printf("\n");

		printf("\t\tChildren:\n");
		printf("\t\t\t");
		for (int i = 0; i < _children.size(); i++) {
			printf("%u ", _children[i]);
		}
		printf("\n");

		printf("\t\tChild Siblings:\n");
		printf("\t\t\t");
		for (int i = 0; i < _child_siblings.size(); i++) {
			printf("%u ", _child_siblings[i]);
		}
		printf("\n\n");
	}

	void WriteDetailsIntoFile(FILE* file) {
		fprintf(file, "\tCluster %d:\n", id);
		fprintf(file, "\t\tIs Leaf: %s\n", (is_leaf ? "true" : "false"));
		fprintf(file, "\t\tIs Root: %s\n", (is_root ? "true" : "false"));
		fprintf(file, "\t\tNeighbours:\n");
		fprintf(file, "\t\t\t");
		for (int i = 0; i < neighbours.size(); i++) {
			fprintf(file, "%u ", neighbours[i]);
		}
		fprintf(file, "\n");

		fprintf(file, "\t\tParents:\n");
		fprintf(file, "\t\t\t");
		for (int i = 0; i < _parents.size(); i++) {
			fprintf(file, "%u ", _parents[i]);
		}
		fprintf(file, "\n");

		fprintf(file, "\t\tParental Siblings:\n");
		fprintf(file, "\t\t\t");
		for (int i = 0; i < _parental_siblings.size(); i++) {
			fprintf(file, "%u ", _parental_siblings[i]);
		}
		fprintf(file, "\n");

		fprintf(file, "\t\tChildren:\n");
		fprintf(file, "\t\t\t");
		for (int i = 0; i < _children.size(); i++) {
			fprintf(file, "%u ", _children[i]);
		}
		fprintf(file, "\n");

		fprintf(file, "\t\tChild Siblings:\n");
		fprintf(file, "\t\t\t");
		for (int i = 0; i < _child_siblings.size(); i++) {
			fprintf(file, "%u ", _child_siblings[i]);
		}
		fprintf(file, "\n\n");
	}

	void Save(const std::string& path, FILE* config_file) {
		if (!OpenMesh::IO::write_mesh(inner_mesh, path))
		{
			std::cerr << "Couldn't save cluster (tough luck)\n";
			return;
		}

		fprintf(config_file, "%u\n", id);
		for (int i = 0; i < _parents.size(); i++) {
			fprintf(config_file, "%u ", _parents[i]);
		}
		fprintf(config_file, "\n");

		for (int i = 0; i < _parental_siblings.size(); i++) {
			fprintf(config_file, "%u ", _parental_siblings[i]);
		}
		fprintf(config_file, "\n");

		for (int i = 0; i < _children.size(); i++) {
			fprintf(config_file, "%u ", _children[i]);
		}
		fprintf(config_file, "\n");

		for (int i = 0; i < _child_siblings.size(); i++) {
			fprintf(config_file, "%u ", _child_siblings[i]);
		}
		fprintf(config_file, "\n");
		fprintf(config_file, "%u\n", (is_leaf ? 1U : 0U));
		fprintf(config_file, "%u\n", (is_root ? 1U : 0U));
		fprintf(config_file, "%f %f %f\n", center.x, center.y, center.z);
	}

	void Load(const std::string& file_path) {
		/*if (!OpenMesh::IO::read_mesh(inner_mesh, file_path))
		{
			std::cerr << "Couldn't load cluster (tough luck)\n";
		}

		inner_mesh.request_vertex_normals();

		inner_mesh.update_face_normals();
		inner_mesh.update_vertex_normals();*/

		inner_static_mesh = StaticMesh(file_path);

		//printf("Cluster data:\n");
		//printf("\tArea: %f\n", area);
		//printf("\tDensity: %f\n", triangle_density);

	}

	void SetParentalDensity(float density) {
		parental_triangle_density = density;
	}

	void SetParentalArea(float area) {
		parental_area = area;
	}

	void SetChildsibDensity(float density) {
		childsib_triangle_density = density;
	}

	void SetChildsibArea(float area) {
		childsib_area = area;
	}

	float GetParentalDensity() {
		return parental_triangle_density;
	}

	float GetParentalArea() {
		return parental_area;
	}

	float GetChildsibDensity() {
		return childsib_triangle_density;
	}

	float GetChildsibArea() {
		return childsib_area;
	}

	float GetSurfaceArea() {
		return inner_static_mesh.GetSurfaceArea();
	}

	void Mark() {
		marked = true;
	}

	void Unmark() {
		marked = false;
	}

	bool IsMarked() {
		return marked;
	}
};