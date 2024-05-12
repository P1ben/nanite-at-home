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
#include "UniformField.h"
#include "Camera.h"
#include "Octree.h"
#include "EdgeCollection.h"
#include "GroupedMesh.h"
#include "Decimator.h"
#include "OpenMeshTest.h"
#include "Decimator2.h"
#include "GraphPartitioner.h"

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#define PROGRAM_NAME "Lod"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

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

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
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

Object3D* triangle = nullptr;
Shader* shader = nullptr;
Scene* scene = nullptr;
Octree* tree = nullptr;

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

void Initialization() {
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glPolygonOffset(1, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glDepthFunc(GL_LESS);
    glLineWidth(1);
    glEnable(GL_MULTISAMPLE);
    glFrontFace(GL_CCW);

    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);

    std::vector<vec3> vertices;
    std::vector<Face> faces;

    vertices.push_back({ -0.5f, -0.5f, 0.0f });
    vertices.push_back({ -0.5f, 0.5f, 0.0f });
    vertices.push_back({ 0.5f, -0.5f, 0.0f });
    vertices.push_back({ 0.5f, 0.5f, 0.0f });

    faces.push_back({ 0, 1, 2 });
    faces.push_back({ 1, 2, 3 });

    //shader = new Shader("shaders/test-vx.glsl", "shaders/test-fg.glsl");
    shader = new Shader("shaders/maxblinn-vx.glsl", "shaders/maxblinn-fg.glsl");


    //humanoid_mesh = OBJParser::Parse("humanoid.obj");
    //humanoid_mesh = new Mesh("humanoid.obj");
    // 
    //humanoid_mesh = new StaticMesh("ment\\output8.obj");
    // 

    //////////// Generate nanite mesh from static mesh ///////////
    //humanoid_mesh = new StaticMesh("dragon.obj");
    //Decimator2 dm2(humanoid_mesh);

    //PRINT_TIME_TAKEN("Creating Nanite Mesh:", {
    //    nanite_mesh = dm2.GetNaniteMesh();
    //})

    //PRINT_TIME_TAKEN("Generating Nanite Mesh:", {
    //    nanite_mesh->Generate();
    //})
    //
    //nanite_mesh->WriteClusterDetailsIntoFile(std::string("logs\\log.txt"));

    //PRINT_TIME_TAKEN("Saving Nanite Mesh:", {
    //    nanite_mesh->Save(std::string("real_nanite_mesh"));
    //})

    //printf("Done\n");
    /////////////////////////////////////////////////////////////

    //////// Render loaded mesh /////////
    PRINT_TIME_TAKEN("Loading Nanite Mesh:", {
        nanite_mesh = new NaniteMesh("real_nanite_mesh");
    })
    
    PRINT_TIME_TAKEN("Setting Step Boundaries:", {
        nanite_mesh->SetChangeStepForClusters(10.2312423f);
    })

    lod_meshes.push_back(nanite_mesh);
    /////////////////////////////////////


    //NaniteMesh* nanite_mesh_small;
    //PRINT_TIME_TAKEN("Grouping Clusters:", {
    //    nanite_mesh_small = nanite_mesh->GroupClusters(4);
    //})



    //PRINT_TIME_TAKEN("Loading Nanite Mesh:", {
    //    lod_meshes.push_back(new NaniteMesh(std::string("nanite_mesh")));
    //})

    //PRINT_TIME_TAKEN("Loading Nanite Mesh Small:", {
    //    lod_meshes.push_back(new NaniteMesh(std::string("nanite_mesh_small")));
    //})

    //PRINT_TIME_TAKEN("Loading Nanite Mesh Small Simple:", {
    //    lod_meshes.push_back(new NaniteMesh(std::string("nanite_mesh_small_simple")));
    //})

    //PRINT_TIME_TAKEN("Decimating Nanite Mesh:", {
    //    nanite_mesh_small->SimplifyClusters();
    //})

    //nanite_mesh_small->Save(std::string("nanite_mesh_small_simple"));

    //nm.PrintClusterDetails();
    //PRINT_TIME_TAKEN("Creating decimator 2:", {
    //    Decimator2 dm2(humanoid_mesh);
    //    printf("Mesh loaded\n");
    //    dm2.MetisTest();
    //    //dm2.CollapseEdges();
    //    printf("Collapse done\n");
    //    std::string out_path = "output.obj";
    //    //dm2.SaveMesh(out_path);
    //})

    /*for (const auto& entry : std::experimental::filesystem::directory_iterator("output"))
        lod_meshes.push_back(new StaticMesh(entry.path().string()));*/

    //lod_meshes.push_back(Octree::SimplifyMesh(humanoid_mesh, ALG_QEF, 0.15f));

    //PRINT_TIME_TAKEN("Creating decimator:", {
    //    Decimator dm = Decimator(humanoid_mesh);
    //})

    //PRINT_TIME_TAKEN("Creating edge collection:", {
    //    EdgeCollection ec = EdgeCollection(humanoid_mesh->GetFaces());
    //    ec.PrintInfo();
    // })
    //tree = new Octree(humanoid_mesh, VecLib::GetAverage(humanoid_mesh->GetVertices()), VecLib::GetBoundingBoxRadius(humanoid_mesh->GetVertices()) + 0.1f, MAX_SIMPLIFICATION_LEVEL);

    //auto start = std::chrono::high_resolution_clock::now();
    ////simplified_mesh_array_simple.push_back(Octree::SimplifyMesh(humanoid_mesh, ALG_SIMPLE_AVG, 1.0f / 5));
    //for (int i = MAX_SIMPLIFICATION_LEVEL; i > MIN_SIMPLIFICATION_LEVEL; i--) {
    //    //simplified_mesh_array_simple.push_back(tree->RestoreMesh(i, ALG_SIMPLE_AVG));
    //    //simplified_mesh_array_qef.push_back(tree->RestoreMesh(i, ALG_QEF));
    //    //simplified_mesh_array_simple.push_back(Octree::SimplifyMesh(humanoid_mesh, ALG_SIMPLE_AVG, 1.0f / i));
    //    //simplified_mesh_array_qef.push_back(Octree::SimplifyMesh(humanoid_mesh, ALG_QEF, 1.0f / i));
    //}
    //auto stop = std::chrono::high_resolution_clock::now();

    //printf("\n20 mesh generated\n");
    //std::cout << "It took " << std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count() / 1e6f << " seconds" << std::endl;

    //PRINT_TIME_TAKEN("Building grouped mesh:", {
    //    GroupedMesh gm = GroupedMesh(humanoid_mesh);
    //    PRINT_TIME_TAKEN("Performing grouping cycle:", {
    //        for (int i = 0; i < 20; i++) {
    //            gm.PerformMergeCycle();
    //        }
    //    })
    //    PRINT_TIME_TAKEN("Printing data:", {
    //        gm.PrintInfoVerbose();
    //    })
    //})


    //PRINT_TIME_TAKEN("Building edge collection", {
    //    EdgeCollection collection = EdgeCollection(facesa);
    //    collection.PrintInfo();
    //    PRINT_TIME_TAKEN("Finding neighbours", {
    //    for (auto neig : collection.GetNeighbours(0U)) {
    //        printf("\t%d\n", neig);
    //    }
    //        })
    //    PRINT_TIME_TAKEN("Finding common point", {
    //    printf("\tCommon point: %u\n", collection.FindCommonPoint(0U, 71384U, 71385U));
    //    })
    //})

    triangle = new Object3D();
    //triangle->SetOriginalMesh(humanoid_mesh);
    triangle->SetOriginalMesh(lod_meshes[0]);

    triangle->SetShader(shader);
    triangle->Material().SetUniform("drawColor", vec3(1.0f, 0.0f, 0));
    //triangle->Material().SetUniform("useTrueColor", false);

    scene = new Scene();
    //scene->SetCamera(vec3(0, 0, -1), vec3(0, 0, 0), vec3(0, 1, 0), 75. * M_PI / 180., (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1, 100);
    scene->SetCamera(vec3(-3.190557, 6.917207, -9.062605), vec3(-0.379691, 4.304642, 0.749835), vec3(0, 1, 0), 75. * M_PI / 180., (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1, 100);
    scene->AddObject(triangle);

    std::vector<std::string> asd = triangle->Material().GetUniforms();

    for (std::string a : asd) {
        printf("%s\n", a.c_str());
    }
}

int current_simplificaton_rate = 0;
int old_simplification_rate = 0;

vec3 current_color = vec3(1.0f, 0, 0);
vec3 old_color = current_color;

Mesh* current_mesh = nullptr;

bool algo_changed = false;
Algorithm current_algo = ALG_SIMPLE_AVG;

/* Our program's entry point */
int main(int argc, char* argv[])
{
    SDL_Window* window; /* Our window handle */
    SDL_GLContext gl_context; /* Our opengl context handle */

    init_window(&window, &gl_context);
    Initialization();

    bool quit = false;

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
                case SDL_BUTTON_MIDDLE:
                    mmb_pressed = true;
                    break;
                }
                break;

            case SDL_MOUSEBUTTONUP:
                switch (event.button.button) {
                case SDL_BUTTON_MIDDLE:
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
                    scene->ZoomCamera(event.wheel.y);
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

        if (mmb_pressed && length(mmb_movement)) {
            if (shift_pressed) {
                scene->MoveCamera(mouse_sensitivity * length(mmb_movement) * 10, mmb_movement);
            }
            else {
                scene->OrbitCamera(M_PI * mouse_sensitivity * length(mmb_movement), mmb_movement);
            }
            mmb_movement = { 0, 0 };
        }

        // Imgui window

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Level of detail");   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        //ImGui::Text("Hello from another window!");
        ImGui::SliderInt("Lod", &current_simplificaton_rate, 0, lod_meshes.size() - 1);
        ImGui::Text("Current algorithm:\n");
        ImGui::Text(current_algo == ALG_QEF ? "QEF" : "Simple Average");
        ImGui::Text("Number of faces:\n");
        ImGui::Text("%d", triangle->GetMesh()->GetFaceCount());
        if (ImGui::Button("Switch method")) {
            if (current_algo == ALG_SIMPLE_AVG) {
                current_algo = ALG_QEF;
            }
            else {
                current_algo = ALG_SIMPLE_AVG;
            }
            algo_changed = true;
        }
        ImGui::End();

        ImGui::Begin("Mesh settings");   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Mesh color:");
        ImGui::SliderFloat("Red", &current_color.x, 0.0f, 1.0f);
        ImGui::SliderFloat("Green", &current_color.y, 0.0f, 1.0f);
        ImGui::SliderFloat("Blue", &current_color.z, 0.0f, 1.0f);

        if (current_color != old_color) {
            old_color = current_color;
            triangle->Material().SetUniform("drawColor", current_color);
        }

        if (ImGui::Button("Toggle Wireframe")) {
            triangle->SwitchWireframe();
        }

        if (ImGui::Button("Toggle TrueColor")) {
            triangle->ToggleTrueColor();
        }

        if (ImGui::Button("Increase Quality")) {
            if (nanite_mesh) {
                nanite_mesh->IncreaseQuality();
            }
        }

        if (ImGui::Button("Decrease Quality")) {
            if (nanite_mesh) {
                nanite_mesh->DecreaseQuality();
            }
        }

        ImGui::End();


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render scene

        if (current_simplificaton_rate != old_simplification_rate || algo_changed) {
            old_simplification_rate = current_simplificaton_rate;
            triangle->SetMesh(lod_meshes[current_simplificaton_rate]);
            algo_changed = false;
        }

        scene->Draw();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    delete scene;
    delete triangle_mesh;
    delete humanoid_mesh;
    delete triangle;
    delete shader;

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