#pragma once
#include <chrono>

class Timer {
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
	uint32_t duration = 0;
public:
	Timer() {}
	void Start(uint32_t _duration) {
		duration = _duration;
		start_time = std::chrono::high_resolution_clock::now();
	}

	bool IsFinished() {
		auto current_time = std::chrono::high_resolution_clock::now();
		auto time_span = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time);
		return time_span.count() >= duration;
	}

	static std::string GetCurrentTimeStr() {
		auto t = std::time(nullptr);
		auto tm = *std::localtime(&t);

		std::ostringstream oss;
		oss << std::put_time(&tm, "%Y-%m-%d-%H-%M-%S");
		return oss.str();
	}
};