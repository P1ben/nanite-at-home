#pragma once
#include <thread>

template<typename T>
class Task {
	T* result;
	bool finished;

	std::thread* running_thread;

public:
	Task() {
		result = nullptr;
		finished = false;
		running_thread = nullptr;
	}

	Task(std::thread* thread) {
		result = nullptr;
		finished = false;
		running_thread = thread;
	}

	void StartJob(std::thread* thread) {
		running_thread = thread;
	}

	bool Finished() {
		return finished;
	}

	T* Result() {
		return result;
	}

	void _SetResult(T* res) {
		result = res;
		finished = true;
	}

	~Task() {
		running_thread->join();
		delete running_thread;
	}
};