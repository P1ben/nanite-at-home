#pragma once
#include "framework.h"
#include "UniformField.h"
#include "Quaternion.h"
#include "Uniform/CameraUniformBlock.h"

class Camera {
private:
	CameraUniformBlock* uniform_block;

	vec3 worldPosition;
	vec3 targetPoint;
	vec3 upDir;

	float fov, asp, fp, bp;

	mat4 viewMatrix = IdentityMatrix();
	mat4 projMatrix = IdentityMatrix();

	vec3 CalculateRotationVector(const vec2& axis);
	vec3 CalculateMovementVector(const vec2& direction);

public:
	Camera(const vec3& pos, const vec3& target, const vec3& up_dir, float fov, float asp, float near_plane, float far_plane);
	~Camera();
	void RecalculateViewMatrix();
	void RecalculateProjectionMatrix();

	void SetWorldPosition(const vec3& pos);
	vec3 GetWorldPosition();

	void Zoom(float amount);
	void Orbit(float radians, const vec2& axis);
	void Move(float amount, const vec2& direction);

	void Use();
};