#include "VecLib.h"
#include <igl/list_to_matrix.h>
#include <iostream>

#define MAX(a, b) a > b ? a : b

vec3 VecLib::GetAverage(const std::vector<vec3>& vectors) {
	vec3 avg = { 0, 0, 0 };

	for (vec3 const& vec : vectors) {
		avg = avg + vec;
	}

	return avg / vectors.size();
}

vec3 VecLib::GetAverage(const std::vector<Vertex>& vectors) {
	vec3 avg = { 0, 0, 0 };

	for (const auto& vec : vectors) {
		avg = avg + vec.position;
	}

	return avg / vectors.size();
}

vec3 VecLib::GetAverage(const RowMatrixf& vertices) {
	vec3 avg = { 0, 0, 0 };

	for (int i = 0; i < vertices.rows(); i++) {
		avg = avg + vec3(vertices(i, 0), vertices(i, 1), vertices(i, 2));
	}

	return avg / vertices.rows();
}

float VecLib::GetBoundingBoxRadius(const std::vector<Vertex>& vertices) {
	vec3 center = VecLib::GetAverage(vertices);
	vec3 min(0, 0, 0), max(0, 0, 0);

	for (const auto& vert : vertices) {
		const vec3& pos = vert.position;
		if (pos.x > max.x) {
			max.x = pos.x;
		}
		else if (pos.x < min.x) {
			min.x = pos.x;
		}

		if (pos.y > max.y) {
			max.y = pos.y;
		}
		else if (pos.y < min.y) {
			min.y = pos.y;
		}

		if (pos.z > max.z) {
			max.z = pos.z;
		}
		else if (pos.z < min.z) {
			min.z = pos.z;
		}
	}
	float max_x = MAX(abs(min.x - center.x), abs(max.x - center.x));
	float max_y = MAX(abs(min.y - center.y), abs(max.y - center.y));
	float max_z = MAX(abs(min.z - center.z), abs(max.z - center.z));

	return MAX(MAX(max_x, max_y), max_z);
}

float VecLib::GetBoundingBoxRadius(const RowMatrixf& vertices) {
	vec3 center = VecLib::GetAverage(vertices);
	vec3 min(0, 0, 0), max(0, 0, 0);

	for (int i = 0; i < vertices.rows(); i++) {
		if (vertices(i, 0) > max.x) {
			max.x = vertices(i, 0);
		}
		else if (vertices(i, 0) < min.x) {
			min.x = vertices(i, 0);
		}

		if (vertices(i, 1) > max.y) {
			max.y = vertices(i, 1);
		}
		else if (vertices(i, 1) < min.y) {
			min.y = vertices(i, 1);
		}

		if (vertices(i, 2) > max.z) {
			max.z = vertices(i, 2);
		}
		else if (vertices(i, 2) < min.z) {
			min.z = vertices(i, 2);
		}
	}
	float max_x = MAX(abs(min.x - center.x), abs(max.x - center.x));
	float max_y = MAX(abs(min.y - center.y), abs(max.y - center.y));
	float max_z = MAX(abs(min.z - center.z), abs(max.z - center.z));

	return MAX(MAX(max_x, max_y), max_z);
}

void VecLib::AddToAll(std::vector<vec3>& vectors, const vec3 vector_to_add) {
	for (vec3& vec : vectors) {
		vec = vec + vector_to_add;
	}
}

void VecLib::MultiplyAll(std::vector<vec3>& vectors, float constant) {
	for (vec3& vec : vectors) {
		vec = vec * constant;
	}
}

void VecLib::CenterAt(std::vector<vec3>& vectors, const vec3 center_point) {
	vec3 avg = GetAverage(vectors);
	AddToAll(vectors, center_point - avg);
}

float VecLib::LargestDistance(std::vector<vec3>& vectors) {
	float largest = 0;

	for (vec3 const& vec_outer : vectors) {
		for (vec3 const& vec_inner : vectors) {
			float dist = length(vec_inner - vec_outer);
			if (dist > largest) {
				largest = dist;
			}
		}
	}

	return largest;
}

void VecLib::ScaleToMaxDistance(std::vector<vec3>& vectors, float max_distance) {
	float old_max_distance = LargestDistance(vectors);
	MultiplyAll(vectors, max_distance / old_max_distance);
}

float VecLib::CalculateSingleErrorByQEF(const vec3& point, const vec3& normal, const vec3& point_on_plane) {
	float error = dot(normal, point - point_on_plane);
	return error * error;
}

float VecLib::CalculateErrorByQEF(const vec3& point, std::vector<vec3>& normals, std::vector<vec3>& points) {
	float error_sum = 0;

	for (int i = 0; i < normals.size(); i++) {
		float error = dot(normals[i], point - points[i]);
		error_sum += error * error;
	}

	return error_sum;
}

vec3 VecLib::GetNewPointByQEF(std::vector<vec3>& normals, std::vector<vec3>& points) {
	std::vector<std::vector<float>> A_vec;
	std::vector<float> B_vec;

	for (int i = 0; i < normals.size(); i++) {
		vec3 point = points[i];
		vec3 normal = normals[i];

		std::vector<float> temp;
		temp.push_back(normal.x);
		temp.push_back(normal.y);
		temp.push_back(normal.z);

		A_vec.push_back(temp);

		float dp = dot(point, normal);

		B_vec.push_back(dp);
	}

	RowMatrixf A;
	RowMatrixf B;

	igl::list_to_matrix(A_vec, A);
	igl::list_to_matrix(B_vec, B);

	// Inverting matrix
	RowMatrixf ATA = (A.transpose() * A);

	Eigen::SelfAdjointEigenSolver<RowMatrixf> solver;
	solver.compute(ATA);

	RowMatrixf e = solver.eigenvectors().real();
	RowMatrixf v = solver.eigenvalues().real();

	float epsilon = 0.0005f;
	Eigen::Matrix<float, 3, 3, Eigen::RowMajor> D;
	D.fill(0.0f);

	//std::cout << D << "\n\n";

	D(0, 0) = v(0, 0) < epsilon ? 0.0f : 1.0f / v(0, 0);
	D(1, 1) = v(1, 0) < epsilon ? 0.0f : 1.0f / v(1, 0);
	D(2, 2) = v(2, 0) < epsilon ? 0.0f : 1.0f / v(2, 0);

	//std::cout << D << "\n";

	RowMatrixf ATAPseudoInv = e * D * e.transpose();

	RowMatrixf vec = (ATAPseudoInv * A.transpose() * B).transpose();
	//printf("\tnew vec rows: %d, cols: %d \n", vec.rows(), vec.cols());
	//std::cout << vec << "\n";

	return vec3(vec(0, 0), vec(0, 1), vec(0, 2));
}

vec3 VecLib::GetNewPointByQEF2(std::vector<vec3>& normals, std::vector<vec3>& points, float weight) {
	float BNX = 0;
	float BNY = 0;
	float BNZ = 0;

	float NXY = 0;
	float NXZ = 0;
	float NYZ = 0;

	float QX = 0;
	float QY = 0;
	float QZ = 0;

	//const float weight = 0.001f;

	for (int i = 0; i < normals.size(); i++) {
		const vec3& normal = normals[i];
		const vec3& point = points[i];

		float dot_prod = dot(normal, point);

		BNX += 2 * dot_prod * normal.x + (2 * weight * point.x);
		BNY += 2 * dot_prod * normal.y + (2 * weight * point.y);
		BNZ += 2 * dot_prod * normal.z + (2 * weight * point.z);

		NXY += 2 * normal.x * normal.y;
		NXZ += 2 * normal.x * normal.z;
		NYZ += 2 * normal.y * normal.z;

		QX += 2 * normal.x * normal.x + (2 * weight);
		QY += 2 * normal.y * normal.y + (2 * weight);
		QZ += 2 * normal.z * normal.z + (2 * weight);
	}

	//float X_denom = ((QY * QZ - NYZ * NYZ) * (QX * QZ - NXZ * NXZ) - (NXZ * NYZ - QZ * NXY) * (NYZ * NXZ - QZ * NXY));
	//if (X_denom < FLT_EPSILON && X_denom > -FLT_EPSILON) {
	//	vec3 avg = vec3(0, 0, 0);
	//	for (const vec3& vec : points) {
	//		avg = avg + vec;
	//	}
	//	avg = avg / points.size();
	//	printf("X denom was 0\n");
	//	//return avg;
	//}
	//float Y_denom = (QY * QZ - NYZ * NYZ);
	//float Z_denom = (QZ);

	float X = (QZ * (QY * QZ - NYZ * NYZ) * BNX - (QY * QZ - NYZ * NYZ) * BNZ * NXZ + (NXZ * NYZ - QZ * NXY) * QZ * BNY - (NXZ * NYZ - QZ * NXY) * NYZ * BNZ) / ((QY * QZ - NYZ * NYZ) * (QX * QZ - NXZ * NXZ) - (NXZ * NYZ - QZ * NXY) * (NYZ * NXZ - QZ * NXY));
	float Y = (QZ * BNY - NYZ * BNZ + (NYZ * NXZ - QZ * NXY) * X) / (QY * QZ - NYZ * NYZ);
	float Z = (BNZ - NXZ * X - NYZ * Y) / (QZ);

	//int same_plane_count = 0;
	//for (int i = 0; i < normals.size(); i++) {
	//	for (int j = 0; j < normals.size(); j++) {
	//		if (normals[i] == normals[j] || normals[i] == normals[j])
	//			same_plane_count += 1;
	//	}
	//}
	//if (same_plane_count > /*normals.size() * normals.size()*/0) {
	//	printf("Num of vertices: %d\n", points.size());
	//	printf("Same plane in number of cases: %d\n\n", same_plane_count - normals.size());
	//}
	//vec3 avg = vec3(0, 0, 0);
	//for (const vec3& vec : points) {
	//	avg = avg + vec;
	//}
	//avg = avg / points.size();
	//if (((QY * QZ - NYZ * NYZ) * (QX * QZ - NXZ * NXZ) - (NXZ * NYZ - QZ * NXY) * (NYZ * NXZ - QZ * NXY)) < 0.2f) {
	//	printf("!!!Possible anomaly!!! (%f)\n", ((QY * QZ - NYZ * NYZ) * (QX * QZ - NXZ * NXZ) - (NXZ * NYZ - QZ * NXY) * (NYZ * NXZ - QZ * NXY)));
	//}
	//bool all_same = true;
	//for (const vec3& a : points) {
	//	if (points[0].x != a.x) {
	//		all_same = false;
	//		break;
	//	}
	//}
	//if (all_same) printf("All x values are the same.\n");
	//float QEF = 0;
	//for (int i = 0; i < normals.size(); i++) {
	//	float err = dot(normals[i], vec3(X, Y, Z) - points[i]);
	//	QEF += err * err;
	//}
	////printf("QEF val: %f\n", QEF);
	//if (length(avg - vec3(X, Y, Z)) > 1.0f) {
	//	printf("Anomaly found:\n");
	//	printf("\tQEF val: %f\n", QEF);
	//	printf("\tCalculated vector: %f %f %f\n", X, Y, Z);
	//	printf("\tAverage vector: %f %f %f\n", avg.x, avg.y, avg.z);
	//	printf("\tDifference: %f\n", length(avg - vec3(X, Y, Z)));
	//	for (const vec3& vec : points) {
	//		printf("\tPoint: %f %f %f\n", vec.x, vec.y, vec.z);
	//	}
	//	for (const vec3& vec : normals) {
	//		printf("\tNormal: %f %f %f\n", vec.x, vec.y, vec.z);
	//	}
	//	printf("Values:\n");
	//	printf("\tX denominator: %f\n", ((QY * QZ - NYZ * NYZ) * (QX * QZ - NXZ * NXZ) - (NXZ * NYZ - QZ * NXY) * (NYZ * NXZ - QZ * NXY)));
	//	printf("\tY denominator: %f\n", (QY * QZ - NYZ * NYZ));
	//	printf("\tZ denominator: %f\n", QZ);
	//	printf("\tBNX: %f\n", BNX);
	//	printf("\tBNY: %f\n", BNY);
	//	printf("\tBNZ: %f\n", BNZ);
	//	printf("\tNXY: %f\n", NXY);
	//	printf("\tNXZ: %f\n", NXZ);
	//	printf("\tNYZ: %f\n", NYZ);
	//	printf("\tQX: %f\n", QX);
	//	printf("\tQY: %f\n", QY);
	//	printf("\tQZ: %f\n", QZ);
	//	//return avg;
	//}

	return vec3(X, Y, Z);
}

vec3 VecLib::CalculateFaceNormal(const vec3& a, const vec3& b, const vec3& c) {
	return normalize(cross((a - b), (b - c)));
}