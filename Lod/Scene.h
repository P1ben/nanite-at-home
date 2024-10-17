#pragma once
#include "framework.h"
#include "Quaternion.h"
#include "Camera.h"
#include "Object3D.h"
#include "UpdateQueue.h"
//#include "ThreadPool/ThreadPool.h"

class Scene {
private:
	Camera*                camera = nullptr;
	std::vector<Object3D*> objects;
	UpdateQueue            update_queue;
	uint32_t               updates_per_frame  = 1;
	bool                   true_color_enabled = false;
	bool                   wireframe_enabled  = false;
	//ThreadPool           thread_pool;

public:
	Scene();
	~Scene();

	void SetCamera(const vec3& pos, const vec3& target, const vec3& up_dir, float fov, float asp, float near_plane, float far_plane);
	void AddObject(Object3D* object);
	void RemoveObject(Object3D* object);
	void ClearAll();
	uint32_t GetVertexCount();
	uint32_t GetFaceCount();
	void SetFreezeViewMatrix(bool freeze);

	void Update();
	void UpdateNow();
	//void UpdateAsync();
	void Draw();

	void ZoomCamera(float amount);
	void OrbitCamera(float rad, const vec2& axis);
	void MoveCamera(float amount, const vec2& dir);
	void SetCameraPosition(const vec3& pos);
	vec3 GetCameraPosition();
	void ToggleTrueColor();
	void ToggleWireframe();
};