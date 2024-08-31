#pragma once
#include "framework.h"
#include "Quaternion.h"
#include "Uniform/CameraUniformBlock.h"

class Camera {
private:
	// Uniform block
	CameraUniformBlock* uniform_block;

	// Uniforms
	mat4 viewMatrix = IdentityMatrix();
	mat4 projMatrix = IdentityMatrix();
	vec3 worldPosition;

	// Camera properties
	vec3 targetPoint;
	vec3 upDir;

	// Projection properties
	float fov, asp, fp, bp;

	// Helper functions
	vec3 CalculateRotationVector(const vec2& axis);
	vec3 CalculateMovementVector(const vec2& direction);

public:
	Camera(const vec3& pos, const vec3& target, const vec3& up_dir, float fov, float asp, float near_plane, float far_plane);
	~Camera();

	// Recalculates the view matrix based on the current camera properties 
	// and updates the uniform block
	void RecalculateViewMatrix();

	// Recalculates the projection matrix based on the current camera properties
	// and updates the uniform block
	void RecalculateProjectionMatrix();

	// Sets the camera world position and updates the uniform block
	void SetWorldPosition(const vec3& pos);
	
	// Returns the camera world position
	vec3 GetWorldPosition();

	// Camera movement functions
	void Zoom(float amount);
	void Orbit(float radians, const vec2& axis);
	void Move(float amount, const vec2& direction);

	// Sets the camera as the render target
	void Use();
};