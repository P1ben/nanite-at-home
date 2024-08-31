#include "Camera.h"

/*
*-------------------*
| Private functions |
*-------------------*
*/

vec3 Camera::CalculateRotationVector(const vec2& axis) {
	vec3 forward = worldPosition - targetPoint;
	forward = normalize(forward);

	vec3 target_sideways = cross(upDir, forward);
	target_sideways = normalize(target_sideways);

	vec3 target_upwards = normalize(cross(forward, target_sideways));
	//vec3 target_upwards = (targetPoint + vec3(0, 1, 0)) - targetPoint;
	//vec3 target_sideways = Quaternion::Rotate(target_upwards, M_PI / 4, targetPoint - worldPosition);
	return -target_upwards * axis.x + target_sideways * axis.y;
}

vec3 Camera::CalculateMovementVector(const vec2& direction) {
	vec3 forward = worldPosition - targetPoint;
	forward = normalize(forward);

	vec3 target_sideways = cross(upDir, forward);
	target_sideways = normalize(target_sideways);

	vec3 target_upwards = normalize(cross(forward, target_sideways));
	return -normalize(target_upwards * direction.y + target_sideways * direction.x);
}

/*
*------------------*
| Public functions |
*------------------*
*/

Camera::Camera(const vec3& pos, const vec3& target, const vec3& up_dir, float fov, float asp, float near_plane, float far_plane) {
	uniform_block = new CameraUniformBlock();

	SetWorldPosition(pos);
	targetPoint = target;
	upDir = up_dir;

	this->asp = asp;
	this->fov = fov;
	this->fp = near_plane;
	this->bp = far_plane;

	/*asp = (float)windowWidth / windowHeight;
	fov = 75.0f * (float)M_PI / 180.0f;
	fp = 0.1; bp = 20;*/
	RecalculateViewMatrix();
	RecalculateProjectionMatrix();
}

Camera::~Camera() {
	delete uniform_block;
}

void Camera::RecalculateViewMatrix() {
	viewMatrix = ViewMatrix(worldPosition, targetPoint, upDir);
	uniform_block->SetViewMatrix(viewMatrix);
}

void Camera::RecalculateProjectionMatrix() { // projection matrix
	projMatrix = ProjectionMatrix(fov, asp, fp, bp);
	uniform_block->SetProjMatrix(projMatrix);
}

void Camera::SetWorldPosition(const vec3& pos) {
	worldPosition = pos;
	uniform_block->SetCameraPosition(worldPosition);
}

vec3 Camera::GetWorldPosition() {
	return worldPosition;
}

void Camera::Zoom(float amount) {
	vec3 dir = normalize(targetPoint - worldPosition);
	vec3 new_pos = worldPosition + normalize(dir) * amount;
	if (length(targetPoint - new_pos) < 0.2f) return;
	SetWorldPosition(new_pos);
	//printf("New camera pos: %f, %f, %f\n", worldPosition.x, worldPosition.y, worldPosition.z);
	RecalculateViewMatrix();
}

void Camera::Orbit(float radians, const vec2& axis) {
	this->worldPosition = Quaternion::Rotate(this->worldPosition, radians, CalculateRotationVector(axis));
	//printf("Camera pos: %f, %f, %f\n", worldPosition.x, worldPosition.y, worldPosition.z);
	RecalculateViewMatrix();
}

void Camera::Move(float amount, const vec2& direction) {
	vec3 dir_vec = CalculateMovementVector(direction);
	this->targetPoint = this->targetPoint + amount * dir_vec;
	SetWorldPosition(this->worldPosition + amount * dir_vec);
	printf("Move.");
	//printf("Camera pos: %f, %f, %f\n", worldPosition.x, worldPosition.y, worldPosition.z);
	//printf("Lookat pos: %f, %f, %f\n", targetPoint.x, targetPoint.y, targetPoint.z);
	RecalculateViewMatrix();
}

void Camera::Use() {
	uniform_block->Bind();
}
