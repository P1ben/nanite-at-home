#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <experimental/filesystem>

#include "../Scene.h"
#include "../Metrics/Timer.h"
#include "../Metrics/FPSCounter.h"
#include "../Metrics/ImageErrorCalculator.h"
#include "../Framebuffer/Framebuffer.h"
#include "../framework.h"
#include "CSVWriter.h"

static constexpr const char* BENCHMARK_SAVE_PATH = "benchmarks\\";

class Benchmark {
private:
	static void CreateFolder(const std::string& folder) {
		std::experimental::filesystem::create_directory(folder);
	}
public:

	static void StationaryFPSBenchmark(SDL_Window*        window,
									   Object3D*          nanite_obj,
									   Object3D*          original_obj,
									   float              sample_time,
									   std::vector<float> distances,
									   int                copy_count,
									   const char*        custom_folder = nullptr)
	{
		std::vector<float> nanite_fps;
		std::vector<float> original_fps;

		std::vector<uint32_t> nanite_mesh_triangle_count;
		std::vector<uint32_t> nanite_mesh_vertex_count;
		std::vector<uint32_t> original_mesh_triangle_count;
		std::vector<uint32_t> original_mesh_vertex_count;

		std::cout << "Starting Stationary FPS Benchmark" << std::endl;

		Scene test_scene;
		test_scene.SetCamera(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 1, 0), 75. * M_PI / 180., (float)1280 / (float)720, 0.1, 100);

		std::vector<Object3D*> nanite_objs;
		for (int i = 0; i < copy_count; i++) {
			nanite_objs.push_back(new Object3D(*nanite_obj));
			nanite_objs.back()->SetWorldPosition(vec3(-1.f * i, .0f, .0f));
			test_scene.AddObject(nanite_objs.back());
		}

		Timer timer;
		FPSCounter fps_counter;

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		std::cout << "Starting Nanite benchmark" << std::endl;
		for (float dist : distances) {
			test_scene.SetCameraPosition(vec3(dist, 0, 0));
			test_scene.Update();

			// init draw call so the buffer gets filled
			test_scene.Draw();
			fps_counter.Start();
			timer.Start(sample_time);
			while (!timer.IsFinished()) {
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				test_scene.Draw();
				SDL_GL_SwapWindow(window);
				fps_counter.Update();
			}
			nanite_fps.push_back(fps_counter.GetFPSAvg());
			Mesh* nanite_mesh = nanite_obj->GetMesh();
			nanite_mesh_triangle_count.push_back(test_scene.GetFaceCount());
			nanite_mesh_vertex_count.push_back(test_scene.GetVertexCount());
			std::cout << "\tNanite FPS at distance " << dist << " is " << nanite_fps.back() << std::endl;
			fps_counter.Reset();
		}

		test_scene.ClearAll();

		for (auto* obj : nanite_objs) {
			delete obj;
		}

		std::vector<Object3D*> reference_objs;
		for (int i = 0; i < copy_count; i++) {
			reference_objs.push_back(new Object3D(*original_obj));
			reference_objs.back()->SetWorldPosition(vec3(-1.f * i, .0f, .0f));
			test_scene.AddObject(reference_objs.back());
		}

		fps_counter.Reset();

		std::cout << "Starting Reference benchmark" << std::endl;
		for (float dist : distances) {
			test_scene.SetCameraPosition(vec3(dist, 0, 0));
			test_scene.Update();
			test_scene.Draw();
			fps_counter.Start();
			timer.Start(sample_time);
			while (!timer.IsFinished()) {
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				test_scene.Draw();
				SDL_GL_SwapWindow(window);
				fps_counter.Update();
			}
			original_fps.push_back(fps_counter.GetFPSAvg());
			Mesh* original_mesh = original_obj->GetMesh();
			original_mesh_triangle_count.push_back(test_scene.GetFaceCount());
			original_mesh_vertex_count.push_back(test_scene.GetVertexCount());
			std::cout << "\tReference FPS at distance " << dist << " is " << original_fps.back() << std::endl;
			fps_counter.Reset();
		}

		for (auto* obj : reference_objs) {
			delete obj;
		}

		std::string date_str;

		if (custom_folder) {
			date_str = custom_folder;
		}
		else {
			date_str = Timer::GetCurrentTimeStr();
		}

		std::string filename = "StationaryFPSBenchmark_" + date_str + ".csv";
		std::string folder   = BENCHMARK_SAVE_PATH + date_str;

		CreateFolder(folder.c_str());

		std::string save_path = folder + "\\" + filename;

		std::vector<std::vector<std::string>> data;
		data.push_back({ "Distance",
						 "Nanite FPS",
						 "Reference FPS",
						 "Nanite Triangle Count",
						 "Reference Triangle Count",
						 "Nanite Vertex Count",
						 "Reference Vertex Count",
					  });

		for (int i = 0; i < distances.size(); i++) {
			data.push_back({ std::to_string(distances[i]),
							 std::to_string(nanite_fps[i]),
							 std::to_string(original_fps[i]),
							 std::to_string(nanite_mesh_triangle_count[i]),
							 std::to_string(original_mesh_triangle_count[i]),
							 std::to_string(nanite_mesh_vertex_count[i]),
							 std::to_string(original_mesh_vertex_count[i]),
						  });
		}

		CSVWriter::WriteCSV(save_path, data);
	}

	static void ImageDifferenceBenchmark(Object3D*          nanite_obj,
										 Object3D*          original_obj,
										 std::vector<float> distances,
										 const char*        custom_folder = nullptr)
	{
		uint32_t img_w = 1920;
		uint32_t img_h = 1080;
		std::string date_str;

		if (custom_folder) {
			date_str = custom_folder;
		}
		else {
			date_str = Timer::GetCurrentTimeStr();
		}

		std::string folder = BENCHMARK_SAVE_PATH + date_str;

		CreateFolder(folder.c_str());

		std::cout << "Starting Image Difference Benchmark" << std::endl;

		Scene test_scene;
		test_scene.SetCamera(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 1, 0), 75. * M_PI / 180., (float)1920 / (float)1080, 0.1, 100);

		test_scene.AddObject(nanite_obj);

		std::vector<double>   errors;
		std::vector<uint32_t> nanite_mesh_triangle_count;
		std::vector<uint32_t> nanite_mesh_vertex_count;
		std::vector<uint32_t> original_mesh_triangle_count;
		std::vector<uint32_t> original_mesh_vertex_count;

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		std::cout << "Starting Nanite benchmark" << std::endl;
		for (float dist : distances) {
			test_scene.SetCameraPosition(vec3(dist, 0, 0));
			test_scene.Update();
			std::string result_file = folder + "\\" + "nanite_" + std::to_string(dist) + ".png";
			
			Framebuffer::RenderOntoImage(&test_scene, result_file.c_str(), img_w, img_h);
			Mesh* nanite_mesh = nanite_obj->GetMesh();
			nanite_mesh_triangle_count.push_back(nanite_mesh->GetFaceCount());
			nanite_mesh_vertex_count.push_back(nanite_mesh->GetVertices().size());
			std::cout << "\tSaved result to: " << result_file << std::endl;
		}

		test_scene.RemoveObject(nanite_obj);
		test_scene.AddObject(original_obj);

		std::cout << "Starting Reference benchmark" << std::endl;
		for (float dist : distances) {
			test_scene.SetCameraPosition(vec3(dist, 0, 0));
			test_scene.Update();
			std::string result_file = folder + "\\" + "reference_" + std::to_string(dist) + ".png";

			Framebuffer::RenderOntoImage(&test_scene, result_file.c_str(), img_w, img_h);
			Mesh* original_mesh = original_obj->GetMesh();
			original_mesh_triangle_count.push_back(original_mesh->GetFaceCount());
			original_mesh_vertex_count.push_back(original_mesh->GetVertices().size());
			std::cout << "\tSaved result to: " << result_file << std::endl;
		}

		for (float dist : distances) {
			std::string nanite_file = folder + "\\" + "nanite_" + std::to_string(dist) + ".png";
			std::string reference_file = folder + "\\" + "reference_" + std::to_string(dist) + ".png";
			std::string diff_file = folder + "\\" + "diff_" + std::to_string(dist) + ".png";

			double error = ImageErrorCalculator::CalculateError(nanite_file.c_str(), reference_file.c_str(), diff_file.c_str());
			std::cout << "\tSaved difference image to: " << diff_file << std::endl;
			std::cout << "\tError at distance " << dist << " is " << error << std::endl;
			errors.push_back(error);
		}

		std::string filename = "ImageDifferenceBenchmark_" + date_str + ".csv";

		CreateFolder(folder.c_str());

		std::string save_path = folder + "\\" + filename;

		std::vector<std::vector<std::string>> data;
		data.push_back({ "Distance",
						 "Error",
						 "Nanite Triangle Count",
						 "Reference Triangle Count",
						 "Nanite Vertex Count",
						 "Reference Vertex Count",
					  });

		for (int i = 0; i < distances.size(); i++) {
			data.push_back({ std::to_string(distances[i]),
							 std::to_string(errors[i]),
							 std::to_string(nanite_mesh_triangle_count[i]),
							 std::to_string(original_mesh_triangle_count[i]),
							 std::to_string(nanite_mesh_vertex_count[i]),
							 std::to_string(original_mesh_vertex_count[i]),
						  });
		}

		CSVWriter::WriteCSV(save_path, data);
	}
};