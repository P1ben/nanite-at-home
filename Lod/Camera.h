#pragma once
#include "framework.h"
#include "UniformField.h"
#include "Quaternion.h"

class Scene {
private:
	class Camera {
		vec3 worldPosition;
		vec3 targetPoint;
		vec3 upDir;

		float fov, asp, fp, bp;

		Scene* parentScene = nullptr;

		mat4 viewMatrix = IdentityMatrix();
		mat4 projMatrix = IdentityMatrix();

	public:
		Camera(const vec3& pos, const vec3& target, const vec3& up_dir, float fov, float asp, float near_plane, float far_plane) {
			worldPosition = pos;
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

		void RecalculateViewMatrix() {
			viewMatrix = ViewMatrix(worldPosition, targetPoint, upDir);
			UpdateObs();
		}

		void RecalculateProjectionMatrix() { // projection matrix
			projMatrix = ProjectionMatrix(fov, asp, fp, bp);
			UpdateObs();
		}

		void SetScene(Scene* scene) {
			parentScene = scene;
		}

		vec3 GetWorldPosition() {
			return worldPosition;
		}

		void UpdateObs() {
			if (parentScene)
				parentScene->UpdateAllMatrices(viewMatrix, projMatrix);
		}

		void Zoom(float amount) {
			vec3 dir = normalize(targetPoint - worldPosition);
			vec3 new_pos = worldPosition + normalize(dir) * amount;
			if (length(targetPoint - new_pos) < 0.2f) return;
			this->worldPosition = new_pos;
			//printf("New camera pos: %f, %f, %f\n", worldPosition.x, worldPosition.y, worldPosition.z);
			RecalculateViewMatrix();
		}
		
		vec3 CalculateRotationVector(const vec2& axis) {
			vec3 forward = worldPosition - targetPoint;
			forward = normalize(forward);

			vec3 target_sideways = cross(upDir, forward);
			target_sideways = normalize(target_sideways);

			vec3 target_upwards = normalize(cross(forward, target_sideways));
			//vec3 target_upwards = (targetPoint + vec3(0, 1, 0)) - targetPoint;
			//vec3 target_sideways = Quaternion::Rotate(target_upwards, M_PI / 4, targetPoint - worldPosition);
			return -target_upwards * axis.x + target_sideways * axis.y;
		}

		vec3 CalculateMovementVector(const vec2& direction) {
			vec3 forward = worldPosition - targetPoint;
			forward = normalize(forward);

			vec3 target_sideways = cross(upDir, forward);
			target_sideways = normalize(target_sideways);

			vec3 target_upwards = normalize(cross(forward, target_sideways));
			return -normalize(target_upwards * direction.y + target_sideways * direction.x);
		}

		void Orbit(float radians, const vec2& axis) {
			this->worldPosition = Quaternion::Rotate(this->worldPosition, radians, CalculateRotationVector(axis));
			//printf("Camera pos: %f, %f, %f\n", worldPosition.x, worldPosition.y, worldPosition.z);
			RecalculateViewMatrix();
		}

		void Move(float amount, const vec2& direction) {
			vec3 dir_vec = CalculateMovementVector(direction);
			this->worldPosition = this->worldPosition + amount * dir_vec;
			this->targetPoint = this->targetPoint + amount * dir_vec;
			//printf("Camera pos: %f, %f, %f\n", worldPosition.x, worldPosition.y, worldPosition.z);
			//printf("Lookat pos: %f, %f, %f\n", targetPoint.x, targetPoint.y, targetPoint.z);
			RecalculateViewMatrix();
		}
	};

	Camera* camera = nullptr;
	std::vector<Object3D*> objects;

public:
	Scene() {}

	void SetCamera(const vec3& pos, const vec3& target, const vec3& up_dir, float fov, float asp, float near_plane, float far_plane) {
		if (camera) {
			delete camera;
		}

		camera = new Camera(pos, target, up_dir, fov, asp, near_plane, far_plane);
		camera->SetScene(this);
	}

	void UpdateAllMatrices(mat4& view_mat, mat4& project_mat) {
		for (auto obj : objects) {
			obj->UpdateModelMVP(view_mat, project_mat);

			if (camera) {
				vec3 camera_pos = camera->GetWorldPosition();
				obj->UpdateMesh(camera_pos);
				obj->UpdateCameraPosition(camera_pos);
			}
		}
	}

	void AddObject(Object3D* object) {
		objects.push_back(object);
		camera->UpdateObs();
	}

	void Draw() {
		for (auto obj : objects) {
			//obj->UpdateLod();
			obj->Draw();
		}
	}

	void ZoomCamera(float amount) {
		if (camera) {
			camera->Zoom(amount);
		}
	}

	void OrbitCamera(float rad, const vec2& axis) {
		if (camera) {
			camera->Orbit(rad, axis);
		}
	}

	void MoveCamera(float amount, const vec2& dir) {
		if (camera) {
			camera->Move(amount, dir);
		}
	}

	~Scene() {
		delete camera;
	}
};