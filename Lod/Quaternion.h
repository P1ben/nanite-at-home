#pragma once
#include "framework.h"

class Quaternion {
private:
	float r;
	vec3  v;

public:
	Quaternion(float real, const vec3& vector) {
		this->r = real;
		this->v = vector;
	}

	Quaternion(float r, float i, float j, float k) {
		this->r = r;
		this->v = { i, j, k };
	}

	Quaternion() :r(.0f), v({0, 0, 0}) {};

	Quaternion Conjugate() const {
		return Quaternion(this->r, -this->v);
	}

	Quaternion Product(const Quaternion& rhs) const {
		return Quaternion(r * rhs.r - v.x * rhs.v.x - v.y * rhs.v.y - v.z * rhs.v.z,
			v.y * rhs.v.z - v.z * rhs.v.y + v.x * rhs.r + r * rhs.v.x,
			v.z * rhs.v.x - v.x * rhs.v.z + v.y * rhs.r + r * rhs.v.y,
			v.x * rhs.v.y - v.y * rhs.v.x + v.z * rhs.r + r * rhs.v.z);
	}

	double Length() const {
		return sqrt(v.x * v.x + v.y * v.y +
			v.z * v.z + r * r);
	}

	Quaternion Inverse(void) const {
		return Conjugate() / Length();
	}

	Quaternion operator * (const Quaternion& rhs) const {
		return Product(rhs);
	}

	Quaternion operator * (double s) const {
		return Quaternion(r * s, v * s);
	}

	Quaternion operator + (const Quaternion& rhs) const {
		return Quaternion(r + rhs.r, v.x + rhs.v.x, v.y + rhs.v.y, v.z + rhs.v.z);
	}

	Quaternion operator - (const Quaternion& rhs) const {
		return Quaternion(r - rhs.r, v.x - rhs.v.x, v.y - rhs.v.y, v.z - rhs.v.z);
	}

	Quaternion operator - () const {
		return Quaternion(-r, -v.x, -v.y, -v.z);
	}

	Quaternion operator / (double s) const {
		if (s == 0) return *this;
		return Quaternion(r / s, v / s);
	}

	vec3 RotatedVector(const vec3& v) const {
		return (((*this) * Quaternion(.0f, v)) * Conjugate()).v;
	}

	static vec3 Rotate(const vec3& point, float radians, const vec3& axis) {
		Quaternion rotation = Quaternion(cosf(radians), sinf(radians) * normalize(axis));
		return rotation.RotatedVector(point);
	}
};