#pragma once
#include "Camera.h"
#include "Object3d.h"
class Camera;

class Scene {
private:
	Camera* camera = nullptr;
	std::vector<Object3D> objects;

public:
	Scene() {}

	void SetCamera(vec3& pos, vec3& target, vec3& up_dir, float fov, float asp, float near_plane, float far_plane) {
		if (camera) {
			delete camera;
		}

		camera = new Camera(pos, target, up_dir, fov, asp, near_plane, far_plane);
	}
};