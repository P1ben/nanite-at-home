#pragma once
#include "Object3D.h"
#include <queue>


class UpdateQueue {
private:
	std::queue<Object3D*> queue;
public:
	UpdateQueue() {}

	void Add(Object3D* object) {
		queue.push(object);
	}

	Object3D* GetNext() {
		if (queue.size() == 0) {
			return nullptr;
		}

		Object3D* object = queue.front();
		queue.pop();
		return object;
	}

	size_t Size() {
		return queue.size();
	}

	void Clear() {
		while (queue.size() > 0) {
			queue.pop();
		}
	}

	void Remove(Object3D* object) {
		std::queue<Object3D*> temp_queue;
		while (queue.size() > 0) {
			Object3D* obj = queue.front();
			queue.pop();
			if (obj != object) {
				temp_queue.push(obj);
			}
		}
		queue = temp_queue;
	}
};