#include <iostream>
#include "framework.h"
#include <glew.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <chrono>
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>

#include "Face.h"
#include "StaticMesh.h"
#include "Nanite/NaniteMesh.h"
#include "Object3D.h"
#include "OBJParser/OBJParser.h"
#include "Shader.h"
#include "Scene.h"
#include "Octree.h"
#include "EdgeCollection.h"
#include "GroupedMesh.h"
#include "Decimator.h"
#include "OpenMeshTest.h"
#include "Decimator2.h"
#include "GraphPartitioner.h"
#include "Framebuffer/Framebuffer.h"
#include "Texture/ObjectSpaceNormalMap.h"
#include "ObjReader/ObjReader.h"
#include "Metrics/FPSCounter.h"
#include "Metrics/ImageErrorCalculator.h"
#include "Benchmark/Benchmark.h"
#include "Compute/NaniteRenderer.h"

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#define PROGRAM_NAME "Lod"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

struct DefaultMeshArgs {
    float scale;
    vec3 rotation_axis;
    float rotation_angle;

    DefaultMeshArgs(float _scale, vec3 _rotation_axis, float _rotation_angle) :
		scale(_scale),
		rotation_axis(_rotation_axis),
		rotation_angle(_rotation_angle) {}
};

struct MeshInfo {
    std::string name;
	std::string nanite_path;
    std::string reference_path;
    std::string reference_texture_path;
    DefaultMeshArgs args;

    MeshInfo(const std::string& _name,
             const std::string& _nanite_path,
             const std::string& _reference_path,
             const std::string& _reference_texture_path,
             const DefaultMeshArgs& _args) :
		     name(_name),
             nanite_path(_nanite_path),
             reference_path(_reference_path),
             reference_texture_path(_reference_texture_path),
             args(_args) {}
};

struct MeshInfoContainer {
    std::vector<MeshInfo> mesh_infos;

    void AddMeshInfo(const std::string& name, const std::string& nanite_path, const std::string& reference_path, const std::string& reference_texture_path, const DefaultMeshArgs& args = DefaultMeshArgs(1.0f, vec3(.0f, .0f, .0f), .0f)) {
		mesh_infos.push_back(MeshInfo(name, nanite_path, reference_path, reference_texture_path, args));
	}

    MeshInfo* GetMeshInfo(const std::string& name) {
		for (auto& mesh_info : mesh_infos) {
			if (mesh_info.name == name) {
				return &mesh_info;
			}
		}
		return nullptr;
	}

    std::vector<const char*> GetMeshNames() {
        std::vector<const char*> names;
        for (auto& mesh_info : mesh_infos) {
            names.push_back(mesh_info.name.c_str());
        }
        return names;
    }
};

void sdldie(const char* msg)
{
    printf("%s: %s\n", msg, SDL_GetError());
    SDL_Quit();
    exit(1);
}


void checkSDLError(int line = -1)
{
#ifndef NDEBUG
    const char* error = SDL_GetError();
    if (*error != '\0')
    {
        printf("SDL Error: %s\n", error);
        if (line != -1)
            printf(" + line: %i\n", line);
        SDL_ClearError();
    }
#endif
}

void init_window(SDL_Window** window, SDL_GLContext* context) {
    SDL_SetMainReady();
    SDL_Window* mainwindow; /* Our window handle */
    SDL_GLContext maincontext; /* Our opengl context handle */

    if (SDL_Init(SDL_INIT_VIDEO) < 0) /* Initialize SDL's Video subsystem */
        sdldie("Unable to initialize SDL"); /* Or die on error */

    /* Request opengl 3.2 context.
     * SDL doesn't have the ability to choose which profile at this time of writing,
     * but it should default to the core profile */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    /* Turn on double buffering with a 24bit Z buffer.
     * You may need to change this to 16 or 32 for your system */
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    /* Create our window centered at 512x512 resolution */
    mainwindow = SDL_CreateWindow(PROGRAM_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!mainwindow) /* Die if creation failed */
        sdldie("Unable to create window");

    checkSDLError(__LINE__);

    /* Create our opengl context and attach it to our window */
    maincontext = SDL_GL_CreateContext(mainwindow);
    checkSDLError(__LINE__);

    /* This makes our buffer swap syncronized with the monitor's vertical refresh */
    SDL_GL_SetSwapInterval(1);

    GLint GlewInitResult = glewInit();
    if (GLEW_OK != GlewInitResult)
    {
        printf("ERROR: %s", glewGetErrorString(GlewInitResult));
        exit(EXIT_FAILURE);
    }

#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(mainwindow, maincontext);
    ImGui_ImplOpenGL3_Init();

    *window = mainwindow;
    *context = maincontext;
}

Mesh* triangle_mesh = nullptr;
Mesh* humanoid_mesh = nullptr;
NaniteMesh* nanite_mesh = nullptr;

Object3D* nanite_obj    = nullptr;
Object3D* nanite_obj2   = nullptr;
Object3D* reference_obj = nullptr;
Shader* maxblinn_shader = nullptr;
Scene* scene = nullptr;
static MeshInfoContainer mesh_info_container;
//Octree* tree = nullptr;

bool mmb_pressed = false;
bool shift_pressed = false;

float mouse_sensitivity = 0.002f;
vec2 mmb_last_pos = { 0, 0 };
vec2 mmb_movement = { 0, 0 };

static const int MAX_SIMPLIFICATION_LEVEL = 3;
static const int MIN_SIMPLIFICATION_LEVEL = 2;
std::vector<Mesh*> simplified_mesh_array_simple;
std::vector<Mesh*> simplified_mesh_array_qef;
std::vector<Mesh*> lod_meshes;

std::vector<float> distances;

void Initialization() {
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glPolygonOffset(1, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glDepthFunc(GL_LESS);
    glLineWidth(1);
    //glEnable(GL_MULTISAMPLE);
    glFrontFace(GL_CCW);
    SDL_GL_SetSwapInterval(0);

    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    //shader = new Shader("shaders/test-vx.glsl", "shaders/test-fg.glsl");
    
    // Max Blinn shader (plz use this)
    maxblinn_shader = new Shader("shaders/maxblinn-vx.glsl", "shaders/maxblinn-fg.glsl");

    //PRINT_TIME_TAKEN("Zipping vase mesh", {
    //    ObjReader::ZipNaniteMesh("vase_nanite");
    //})

    //Decimator2::CreateNaniteMesh("vase.obj", "vase_test");

	// Init mesh info container
	DefaultMeshArgs vase_args = DefaultMeshArgs(0.002f, vec3(1.f, .0f, .0f), -M_PI / 2.f);
    mesh_info_container.AddMeshInfo("Vase", "vase_nanite", "vase.obj", "vase.jpg", vase_args);

    DefaultMeshArgs axe_args = DefaultMeshArgs(0.02f, vec3(1.f, .0f, .0f), -M_PI / 2.f);
    mesh_info_container.AddMeshInfo("Axe", "axe_nanite", "axe.obj", "axe_texture.png", axe_args);

    DefaultMeshArgs chair_args = DefaultMeshArgs(0.002f, vec3(1.f, .0f, .0f), -M_PI / 2.f);
    mesh_info_container.AddMeshInfo("Chair", "chair_nanite", "chair.obj", "chair.png", chair_args);

    scene = new Scene();
    //scene->SetCamera(vec3(0, 0, -1), vec3(0, 0, 0), vec3(0, 1, 0), 75. * M_PI / 180., (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1, 100);
    scene->SetCamera(vec3(-3, 0, 0), vec3(0, 0, 0), vec3(0, 1, 0), 75. * M_PI / 180., (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1, 100);
}

int current_simplificaton_rate = 0;
int old_simplification_rate = 0;

vec3 current_color = vec3(1.0f, 0, 0);
vec3 old_color = current_color;

Mesh* current_mesh = nullptr;

bool algo_changed = false;
//Algorithm current_algo = ALG_SIMPLE_AVG;

int  frames_elapsed = 0;
bool animation_started = false;

bool camera_frozen = false;

FPSCounter fps_counter;

// Benchmark settings

static int   BENCHMARK_STATIONARY_SAMPLE_TIME  =     2;
static int   BENCHMARK_UNIVERSAL_COPY_COUNT    =     5;
static int   BENCHMARK_UNIVERSAL_COPY_DISTANCE =     1;
static int   BENCHMARK_MOVING_ANIMATION_TIME   =    10;
static float BENCHMARK_START_DISTANCE          =  1.0f;
static float BENCHMARK_END_DISTANCE            = 20.0f;
static float BENCHMARK_DISTANCE_STEP           =  0.5f;
static std::string LOADED_MESH_NAME            = "N/A";

void RecalculateDistances() {
	distances.clear();
	for (float i = BENCHMARK_START_DISTANCE; i <= BENCHMARK_END_DISTANCE; i += BENCHMARK_DISTANCE_STEP) {
		distances.push_back(i);
	}
}

std::vector<std::string> GetBenchmarkSettings() {
	std::vector<std::string> settings;
	settings.push_back("Stationary sample time: " + std::to_string(BENCHMARK_STATIONARY_SAMPLE_TIME) + " s");
	settings.push_back("Universal copy count: " + std::to_string(BENCHMARK_UNIVERSAL_COPY_COUNT));
	settings.push_back("Universal copy distance: " + std::to_string(BENCHMARK_UNIVERSAL_COPY_DISTANCE));
    settings.push_back("Moving animation time: " + std::to_string(BENCHMARK_MOVING_ANIMATION_TIME) + " s");
	settings.push_back("Number of distances: " + std::to_string(distances.size()));
    settings.push_back("Start distance: " + std::to_string(BENCHMARK_START_DISTANCE));
    settings.push_back("End distance: " + std::to_string(BENCHMARK_END_DISTANCE));
    settings.push_back("Distance step: " + std::to_string(BENCHMARK_DISTANCE_STEP));
    settings.push_back("Loaded mesh: " + std::string(LOADED_MESH_NAME));
	return settings;
}

void SaveEnvironmentInfo(const std::string& folder_name) {
    auto sys_info = Benchmark::GetSysInfo();
    auto benchmark_settings = GetBenchmarkSettings();

    std::vector<std::string> all_settings;
    all_settings.insert(all_settings.end(), sys_info.begin(), sys_info.end());
    all_settings.insert(all_settings.end(), benchmark_settings.begin(), benchmark_settings.end());

    std::ofstream info_file(std::string("benchmarks\\") + folder_name + "\\env_info.txt");
    for (const auto& e : all_settings) info_file << e << "\n";
}

/* Our program's entry point */
int main(int argc, char* argv[])
{
    SDL_Window* window; /* Our window handle */
    SDL_GLContext gl_context; /* Our opengl context handle */

    init_window(&window, &gl_context);
    Initialization();

    bool quit = false;
    std::vector<const char*> mesh_item_names = mesh_info_container.GetMeshNames();
    int selected_mesh_id = 0; // you need to store this state somewhere

    fps_counter.Start();

    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // Handle stuff
            ImGui_ImplSDL2_ProcessEvent(&event);
            switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                case SDLK_LSHIFT:
                    shift_pressed = true;
                    break;
                case SDLK_a:
                    animation_started = !animation_started;
                    break;
                }
                break;

            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                case SDLK_LSHIFT:
                    shift_pressed = false;
                    break;
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                switch (event.button.button) {
                case SDL_BUTTON_RIGHT:
                    mmb_pressed = true;
                    break;
                }
                break;

            case SDL_MOUSEBUTTONUP:
                switch (event.button.button) {
                case SDL_BUTTON_RIGHT:
                    mmb_pressed = false;
                    break;
                }
                break;

            case SDL_MOUSEMOTION:
            {
                mmb_movement.x = event.motion.xrel;
                mmb_movement.y = -event.motion.yrel;
                break;
            }

            case SDL_MOUSEWHEEL:
                if (event.wheel.y != 0) // scroll up
                {
                    scene->ZoomCamera(event.wheel.y * 0.2f);
                }
                break;

            case SDL_QUIT:
            {
                quit = true;
                break;
            }
            }
        }
        // Draw
        //glClearColor(0.5f, 0.5f, 0.8f, 1.0f);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        //glClearColor(0.f, 0.f, 0.f, 1.0f);

        if (mmb_pressed && length(mmb_movement)) {
            if (shift_pressed) {
                scene->MoveCamera(mouse_sensitivity * length(mmb_movement) * fps_counter.GetLastFrametime(), mmb_movement);
            }
            else {
                scene->OrbitCamera(mouse_sensitivity * length(mmb_movement) * fps_counter.GetLastFrametime(), mmb_movement);
            }
            mmb_movement = { 0, 0 };
        }

        // Imgui window

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();

        ImGui::NewFrame();

        ImGui::Begin("Load mesh");
        ImGui::ListBox("##", &selected_mesh_id, mesh_item_names.data(), mesh_item_names.size(), 4);
        if (ImGui::Button("Load")) {
            MeshInfo* mesh_info = mesh_info_container.GetMeshInfo(mesh_item_names[selected_mesh_id]);
            if (mesh_info) {
                LOADED_MESH_NAME = mesh_info->name;
                if (nanite_obj) {
					scene->RemoveObject(nanite_obj);
                    nanite_obj->Destroy();
					delete nanite_obj;
                    nanite_obj = nullptr;
				}
                if (reference_obj) {
                    reference_obj->Destroy();
					delete reference_obj;
                    reference_obj = nullptr;
				}

                float scale     = mesh_info->args.scale;
                vec3 rot_axis   = mesh_info->args.rotation_axis;
                float rot_angle = mesh_info->args.rotation_angle;

                PRINT_TIME_TAKEN("Loading Nanite Object:", {
                    std::cout << "Loading nanite object..." << std::endl;
                    nanite_obj = Object3D::LoadNaniteObject(mesh_info->nanite_path, true);
                    nanite_obj->SetShader(maxblinn_shader);
                    nanite_obj->SetDrawColor(vec3(1.0f, 0.0f, 0));
                    nanite_obj->SetScale(vec3(scale, scale, scale));
                    nanite_obj->SetRotation(rot_axis, rot_angle);
                })

                PRINT_TIME_TAKEN("Loading Reference Object:", {
                    std::cout << "Loading reference object..." << std::endl;
                    Decimator2 dm2(ObjReader::ReadObj(mesh_info->reference_path.c_str(), true));
                    Mesh* ref_mesh = dm2.ConvertToStaticMesh();
                    reference_obj = new Object3D();
                    reference_obj->SetOriginalMesh(ref_mesh);
                    reference_obj->SetShader(maxblinn_shader);
                    reference_obj->SetDrawColor(vec3(1.0f, 0.0f, 0));
                    reference_obj->AttachColorMap(new ColorMap(mesh_info->reference_texture_path.c_str(), true));
                    reference_obj->SetScale(vec3(scale, scale, scale));
                    reference_obj->SetRotation(rot_axis, rot_angle);
                })
                scene->AddObject(nanite_obj);
            }
        }

        if (ImGui::Button("Unload")) {
            LOADED_MESH_NAME = "N/A";
            if (nanite_obj) {
                scene->RemoveObject(nanite_obj);
                nanite_obj->Destroy();
                delete nanite_obj;
                nanite_obj = nullptr;
            }
            if (reference_obj) {
                reference_obj->Destroy();
                delete reference_obj;
                reference_obj = nullptr;
            }
        }

        ImGui::End();

        ImGui::Begin("Level of detail");
        ImGui::Text("Number of faces:\n");
        ImGui::Text("%d", scene->GetFaceCount());
        ImGui::Text("Number of vertices:\n");
        ImGui::Text("%d", scene->GetVertexCount());
        ImGui::Text("Camera distance:\n");
        ImGui::Text("%.2f", length(scene->GetCameraPosition()));
        /*if (ImGui::Button("Switch method")) {
            if (current_algo == ALG_SIMPLE_AVG) {
                current_algo = ALG_QEF;
            }
            else {
                current_algo = ALG_SIMPLE_AVG;
            }
            algo_changed = true;
        }*/
        ImGui::End();

        ImGui::Begin("Mesh/scene settings");
        //ImGui::Text("Mesh color:");
        /*ImGui::SliderFloat("Red", &current_color.x, 0.0f, 1.0f);
        ImGui::SliderFloat("Green", &current_color.y, 0.0f, 1.0f);
        ImGui::SliderFloat("Blue", &current_color.z, 0.0f, 1.0f);*/

        if (current_color != old_color) {
            old_color = current_color;
            nanite_obj->SetDrawColor(current_color);
        }

        if (ImGui::Button("Toggle Wireframe")) {
            scene->ToggleWireframe();
        }

        if (ImGui::Button("Toggle Cluster Color")) {
            scene->ToggleTrueColor();
        }

        //if (ImGui::Button("Increase Quality")) {
        //    if (nanite_mesh) {
        //        nanite_mesh->IncreaseQuality();
        //    }
        //}

        //if (ImGui::Button("Decrease Quality")) {
        //    if (nanite_mesh) {
        //        nanite_mesh->DecreaseQuality();
        //    }
        //}

        if (ImGui::Button("Freeze/Unfreeze Camera")) {
            camera_frozen = !camera_frozen;
            scene->SetFreezeViewMatrix(camera_frozen);
        }

        if (ImGui::Button("Save UV map")) {
            if (nanite_obj)
                Framebuffer::RenderUVMap(nanite_obj, "uv_map.jpg");
		}

        //ImGui::SliderFloat("Blue", &current_color.z, 0.0f, 1.0f);

        ImGui::End();

        ImGui::Begin("Shader");
        if (ImGui::Button("Reload")) {
            if (maxblinn_shader) {
                maxblinn_shader->Reload();
            }
        }
        ImGui::End();

        ImGui::Begin("Performance");

        ImGui::Text("Current FPS:");
        ImGui::Text(std::to_string(fps_counter.GetLastFPS()).c_str());
        ImGui::Text("Average FPS:");
        ImGui::Text(std::to_string(fps_counter.GetFPSAvg()).c_str());
        ImGui::Text("Last Frame Time (ms):");
        ImGui::Text(std::to_string(fps_counter.GetLastFrametime()).c_str());
        ImGui::Text("Stationary sample time (s):");
        ImGui::SliderInt("##Stationary sample time", &BENCHMARK_STATIONARY_SAMPLE_TIME,     1,        10);
        ImGui::Text("Moving animation time (s):");
        ImGui::SliderInt("##Moving animation time",  &BENCHMARK_MOVING_ANIMATION_TIME,      1,        20);
        ImGui::Text("Copy count:");
        ImGui::SliderInt("##Copy count",             &BENCHMARK_UNIVERSAL_COPY_COUNT,       1,       200);
        ImGui::Text("Max triangle count:");
        ImGui::Text((reference_obj ? std::to_string(reference_obj->GetFaceCount() * BENCHMARK_UNIVERSAL_COPY_COUNT) : std::string("0")).c_str());
        ImGui::Text("Copy distance:");
        ImGui::SliderInt("##Copy distance",          &BENCHMARK_UNIVERSAL_COPY_DISTANCE,    1,         5);
        ImGui::Text("Start distance:");
        ImGui::SliderFloat("##Start distance",       &BENCHMARK_START_DISTANCE,             0.1f,  10.0f);
        ImGui::Text("End distance:");
        ImGui::SliderFloat("##End distance",         &BENCHMARK_END_DISTANCE,               11.0f, 50.0f);
        ImGui::Text("Distance step:");
        ImGui::SliderFloat("##Distance step",        &BENCHMARK_DISTANCE_STEP,              0.1f,   2.0f);

        if (ImGui::Button("Start Stationary FPS Benchmark")) {
            RecalculateDistances();
            std::string date_str = Timer::GetCurrentTimeStr();
            Benchmark::StationaryFPSBenchmark(window, nanite_obj, reference_obj, BENCHMARK_STATIONARY_SAMPLE_TIME, distances, BENCHMARK_UNIVERSAL_COPY_COUNT, BENCHMARK_UNIVERSAL_COPY_DISTANCE, date_str.c_str());
            SaveEnvironmentInfo(date_str);
        }

        if (ImGui::Button("Start Moving FPS Benchmark")) {
            RecalculateDistances();
            std::string date_str = Timer::GetCurrentTimeStr();
            Benchmark::MovingFPSBenchmark(window, nanite_obj, reference_obj, BENCHMARK_MOVING_ANIMATION_TIME, distances, BENCHMARK_UNIVERSAL_COPY_COUNT, BENCHMARK_UNIVERSAL_COPY_DISTANCE, date_str.c_str());
            SaveEnvironmentInfo(date_str);
        }

        if (ImGui::Button("Start Image Difference Benchmark")) {
            RecalculateDistances();
            std::string date_str = Timer::GetCurrentTimeStr();
            Benchmark::ImageDifferenceBenchmark(nanite_obj, reference_obj, distances, BENCHMARK_UNIVERSAL_COPY_COUNT, BENCHMARK_UNIVERSAL_COPY_DISTANCE, date_str.c_str());
            SaveEnvironmentInfo(date_str);
        }

        if (ImGui::Button("Start Performance Benchmark")) {
            RecalculateDistances();
            std::string date_str = Timer::GetCurrentTimeStr();
            Benchmark::StationaryFPSBenchmark(window, nanite_obj, reference_obj, BENCHMARK_STATIONARY_SAMPLE_TIME, distances, BENCHMARK_UNIVERSAL_COPY_COUNT, BENCHMARK_UNIVERSAL_COPY_DISTANCE, date_str.c_str());
            Benchmark::MovingFPSBenchmark(window, nanite_obj, reference_obj, BENCHMARK_MOVING_ANIMATION_TIME, distances, BENCHMARK_UNIVERSAL_COPY_COUNT, BENCHMARK_UNIVERSAL_COPY_DISTANCE, date_str.c_str());
            SaveEnvironmentInfo(date_str);
        }

        if (ImGui::Button("Start Combined Benchmark")) {
            RecalculateDistances();
            std::string date_str = Timer::GetCurrentTimeStr();
            Benchmark::StationaryFPSBenchmark(window, nanite_obj, reference_obj, BENCHMARK_STATIONARY_SAMPLE_TIME, distances, BENCHMARK_UNIVERSAL_COPY_COUNT, BENCHMARK_UNIVERSAL_COPY_DISTANCE, date_str.c_str());
            Benchmark::ImageDifferenceBenchmark(nanite_obj, reference_obj, distances, BENCHMARK_UNIVERSAL_COPY_COUNT, BENCHMARK_UNIVERSAL_COPY_DISTANCE, date_str.c_str());
            Benchmark::MovingFPSBenchmark(window, nanite_obj, reference_obj, BENCHMARK_MOVING_ANIMATION_TIME, distances, BENCHMARK_UNIVERSAL_COPY_COUNT, BENCHMARK_UNIVERSAL_COPY_DISTANCE, date_str.c_str());
            SaveEnvironmentInfo(date_str);
        }

        ImGui::End();


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render scene

        if (current_simplificaton_rate != old_simplification_rate || algo_changed) {
            old_simplification_rate = current_simplificaton_rate;
            nanite_obj->SetMesh(lod_meshes[current_simplificaton_rate]);
            algo_changed = false;
        }

        if (animation_started)
            scene->ZoomCamera(-0.004f * fps_counter.GetLastFrametime());
        frames_elapsed = 0;

        frames_elapsed++;

        scene->Update();
        scene->Draw();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
        fps_counter.Update();
    }

    delete scene;
    delete triangle_mesh;
    delete humanoid_mesh;
    delete nanite_obj;
    delete reference_obj;
    delete maxblinn_shader;

    for (Mesh* msh : simplified_mesh_array_simple) {
        delete msh;
    }

    for (Mesh* msh : simplified_mesh_array_qef) {
        delete msh;
    }

    //for (Mesh* msh : lod_meshes) {
    //    delete msh;
    //}

    /* Delete our opengl context, destroy our window, and shutdown SDL */
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}