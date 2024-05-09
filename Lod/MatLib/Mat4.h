#pragma once
// in prog

struct mat4 { // row-major matrix 4x4
	//---------------------------
	float data[16];
public:
	mat4()
	{
		data[0] = data[5] = data[10] = data[15] = 1;
	}
	mat4(float m00, float m01, float m02, float m03,
		 float m10, float m11, float m12, float m13,
		 float m20, float m21, float m22, float m23,
		 float m30, float m31, float m32, float m33)
	{
		data[0] = m00; data[1] = m01; data[2] = m02; data[3] = m03;
		data[4] = m10; data[5] = m11; data[6] = m12; data[7] = m13;
		data[8] = m20; data[9] = m21; data[10] = m22; data[11] = m23;
		data[12] = m30; data[13] = m31; data[14] = m32; data[15] = m33;
	}
	mat4(vec4& first_row, vec4& second_row, vec4& third_row, vec4& fourth_row) {
		data[0] = first_row[0]; data[1] = first_row[1]; data[2] = first_row[2]; data[3] = first_row[3];
		data[4] = second_row[0]; data[5] = second_row[1]; data[6] = second_row[2]; data[7] = second_row[3];
		data[8] = third_row[0]; data[9] = third_row[1]; data[10] = third_row[2]; data[11] = third_row[3];
		data[12] = fourth_row[0]; data[13] = fourth_row[1]; data[14] = fourth_row[2]; data[15] = fourth_row[3];
	}

	vec4 GetRow(int index) const {
		return vec4(data[index * 4], data[index * 4 + 1], data[index * 4 + 1], data[index * 4 + 1]);
	}

	void SetRow(int index, vec4 row) {
		data[index * 4] = row[0];
		data[index * 4 + 1] = row[1];
		data[index * 4 + 2] = row[2];
		data[index * 4 + 3] = row[3];
	}

	float& operator[](int i) { return data[i]; }
	float  operator[](int i) const { return data[i]; }
	operator float* () const { return (float*)this; }

	inline mat4 TranslateMatrix(vec3& t) {
		return mat4(vec4(1, 0, 0, 0),
					vec4(0, 1, 0, 0),
					vec4(0, 0, 1, 0),
					vec4(t.x, t.y, t.z, 1));
	}

	inline mat4 IdentityMatrix() {
		return mat4(vec4(1, 0, 0, 0),
					vec4(0, 1, 0, 0),
					vec4(0, 0, 1, 0),
					vec4(0, 0, 0, 1));
	}

	inline mat4 ScaleMatrix(vec3& s) {
		return mat4(vec4(s.x, 0, 0, 0),
					vec4(0, s.y, 0, 0),
					vec4(0, 0, s.z, 0),
					vec4(0, 0, 0, 1));
	}

	inline mat4 RotationMatrix(float angle, vec3& axis) {
		float c = cosf(angle), s = sinf(angle);
		vec3 w = normalize(axis);
		return mat4(vec4(c * (1 - w.x * w.x) + w.x * w.x, w.x * w.y * (1 - c) + w.z * s, w.x * w.z * (1 - c) - w.y * s, 0),
					vec4(w.x * w.y * (1 - c) - w.z * s, c * (1 - w.y * w.y) + w.y * w.y, w.y * w.z * (1 - c) + w.x * s, 0),
					vec4(w.x * w.z * (1 - c) + w.y * s, w.y * w.z * (1 - c) - w.x * s, c * (1 - w.z * w.z) + w.z * w.z, 0),
					vec4(0, 0, 0, 1));
	}
};
