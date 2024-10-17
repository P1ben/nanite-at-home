#pragma once
#include <chrono>


class FPSCounter {
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> a;
	std::chrono::time_point<std::chrono::high_resolution_clock> b;

	bool a_selected = false;
	uint8_t exp_factor = 10;

	float last_frametime = 0.0f;
	float last_fps       = 0.0f;
	float last_fps_avg   = 0.0f;
	float last_fps_exp   = 0.0f;
	uint32_t frame_count = 0;

	void UpdateFPSAvg() {
		float a = 1.0f / frame_count;
		float b = 1.0f - a;
		last_fps_avg = last_fps_avg * b + last_fps * a;
	}

	void UpdateExpMovingAvg() {
		last_fps_exp += (last_fps - last_fps_exp) / ((frame_count > exp_factor) ? exp_factor : frame_count);
	}

public:
	FPSCounter(uint8_t _exp_factor = 5) {
		exp_factor = _exp_factor;
	};

	void Start() {
		if (a_selected) {
			a = std::chrono::high_resolution_clock::now();
		} else {
			b = std::chrono::high_resolution_clock::now();
		}
		a_selected = !a_selected;
	}

	void Update() {
		frame_count++;
		uint32_t delta_time = 0;
		if (a_selected) {
			a = std::chrono::high_resolution_clock::now();
			delta_time = std::chrono::duration_cast<std::chrono::nanoseconds>(a - b).count();
		}
		else {
			b = std::chrono::high_resolution_clock::now();
			delta_time = std::chrono::duration_cast<std::chrono::nanoseconds>(b - a).count();
		}
		last_frametime = delta_time / 1000000.0f;
		if (last_frametime > 0.0f) {
			last_fps = 1000.0f / last_frametime;
		}
		a_selected = !a_selected;

		UpdateFPSAvg();
		UpdateExpMovingAvg();
	}

	float GetLastFrametime() {
		return last_frametime;
	}
	float GetLastFPS() {
		return last_fps;
	}
	float GetFPSAvg() {
		return last_fps_avg;
	}

	float GetExpMovingAvg() {
		return last_fps_exp;
	}

	void Reset() {
		last_fps = 0.0f;
		last_fps_avg = 0.0f;
		last_fps_exp = 0.0f;
		frame_count = 0;
	}
};