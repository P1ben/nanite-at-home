#pragma once
#include "../framework.h"
#include "../Vertex.h"

class VecLib {
public:
	static vec3 GetAverage(const RowMatrixf& vertices);
	static vec3 GetAverage(const std::vector<vec3>& vectors);
	static vec3 GetAverage(const std::vector<Vertex>& vectors);

	static float GetBoundingBoxRadius(const RowMatrixf& vertices);
	static float GetBoundingBoxRadius(const std::vector<Vertex>& vertices);

	static void AddToAll(std::vector<vec3>& vectors, const vec3 vector_to_add);
	static void MultiplyAll(std::vector<vec3>& vectors, float constant);

	static void CenterAt(std::vector<vec3>& vectors, const vec3 center_point);
	static float LargestDistance(std::vector<vec3>& vectors);
	static void ScaleToMaxDistance(std::vector<vec3>& vectors, float max_distance);


	static float CalculateSingleErrorByQEF(const vec3& point, const vec3& normal, const vec3& point_on_plane);
	static float CalculateErrorByQEF(const vec3& point, std::vector<vec3>& normals, std::vector<vec3>& points);
	static vec3 GetNewPointByQEF(std::vector<vec3>& normals, std::vector<vec3>& points);
	static vec3 GetNewPointByQEF2(std::vector<vec3>& normals, std::vector<vec3>& points, float weight);
	static vec3 CalculateFaceNormal(const vec3& a, const vec3& b, const vec3& c);
};