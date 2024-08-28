#pragma once
#include "framework.h"
#include "UniformField.h"
#include "Quaternion.h"
#include "Camera.h"
#include "Object3D.h"

class Scene {
private:
	Camera* camera = nullptr;
	std::vector<Object3D*> objects;

public:
	Scene() {}
	~Scene() {
		delete camera;
	}

	void SetCamera(const vec3& pos, const vec3& target, const vec3& up_dir, float fov, float asp, float near_plane, float far_plane) {
		if (camera) {
			delete camera;
		}

		camera = new Camera(pos, target, up_dir, fov, asp, near_plane, far_plane);
	}

	void UpdateAllMatrices(mat4& view_mat, mat4& project_mat) {
		for (auto obj : objects) {
			obj->UpdateModelMVP(view_mat, project_mat);

			if (camera) {
				vec3 camera_pos = camera->GetWorldPosition();
				obj->UpdateCameraPosition(camera_pos);
				obj->UpdateMesh(camera_pos);
			}
		}
	}

	void AddObject(Object3D* object) {
		objects.push_back(object);
	}

	void Draw() {
		if (!camera) {
			printf("No camera added to the scene, could not initiate draw.");
			return;
		}

		camera->Use();
		for (auto obj : objects) {
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
};