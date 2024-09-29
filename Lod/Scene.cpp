#include "Scene.h"

void Scene::Update()
{
	if (camera) {
		const vec3& camera_pos = camera->GetWorldPosition();
		for (auto obj : objects) {
			obj->UpdateMesh(camera_pos);
		}
	}
}

//void Scene::UpdateAsync()
//{
//	if (camera) {
//		const vec3& camera_pos = camera->GetWorldPosition();
//		thread_pool.Start();
//		for (auto obj : objects) {
//			thread_pool.QueueJob([&](void* _) {
//				obj->UpdateMeshNoRefill(camera_pos);
//			});
//		}
//
//		thread_pool.Join();
//		for (auto obj : objects) {
//			obj->RefillBufferIfNeeded();
//		}
//	}
//}

Scene::Scene() {}

Scene::~Scene() {
	delete camera;
}

void Scene::SetCamera(const vec3& pos, const vec3& target, const vec3& up_dir, float fov, float asp, float near_plane, float far_plane) {
	if (camera) {
		delete camera;
	}

	camera = new Camera(pos, target, up_dir, fov, asp, near_plane, far_plane);
}

void Scene::AddObject(Object3D* object) {
	objects.push_back(object);
}

void Scene::RemoveObject(Object3D* object) {
	auto it = std::find(objects.begin(), objects.end(), object);
	if (it != objects.end()) {
		objects.erase(it);
	}
}

void Scene::ClearAll() {
	objects.clear();
}

uint32_t Scene::GetVertexCount() {
	uint32_t count = 0;
	for (auto obj : objects) {
		count += obj->GetVertexCount();
	}
	return count;
}

uint32_t Scene::GetFaceCount() {
	uint32_t count = 0;
	for (auto obj : objects) {
		count += obj->GetFaceCount();
	}
	return count;
}

void Scene::SetFreezeViewMatrix(bool freeze) {
	if (camera) {
		camera->SetFreezeViewMatrix(freeze);
	}
}

void Scene::Draw() {
	if (!camera) {
		printf("No camera added to the scene, could not initiate draw.");
		return;
	}

	camera->Use();
	for (auto obj : objects) {
		obj->Draw();
	}
}

void Scene::ZoomCamera(float amount) {
	if (camera) {
		camera->Zoom(amount);
	}
}

void Scene::OrbitCamera(float rad, const vec2& axis) {
	if (camera) {
		camera->Orbit(rad, axis);
	}
}

void Scene::MoveCamera(float amount, const vec2& dir) {
	if (camera) {
		camera->Move(amount, dir);
	}
}

void Scene::SetCameraPosition(const vec3& pos) {
	if (camera) {
		camera->SetWorldPosition(pos);
	}
}
