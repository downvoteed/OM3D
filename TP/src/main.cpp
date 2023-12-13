#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <graphics.h>
#include <Scene.h>
#include <Texture.h>
#include <Framebuffer.h>
#include <ImGuiRenderer.h>

#include <imgui/imgui.h>

#include <iostream>
#include <vector>
#include <filesystem>

using namespace OM3D;

static float delta_time = 0.0f;
static std::unique_ptr<Scene> scene;
static float exposure = 1.0;
static std::vector<std::string> scene_files;

static bool g_buffer_isdepth = false;
static bool g_buffer_isalbedo = false;
static bool g_buffer_isnormal = false;

namespace OM3D {
extern bool audit_bindings_before_draw;
}


void parse_args(int argc, char** argv) {
    for(int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];

        if(arg == "--validate") {
            OM3D::audit_bindings_before_draw = true;
        } else {
            std::cerr << "Unknown argument \"" << arg << "\"" << std::endl;
        }
    }
}

void glfw_check(bool cond) {
    if(!cond) {
        const char* err = nullptr;
        glfwGetError(&err);
        std::cerr << "GLFW error: " << err << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void update_delta_time() {
    static double time = 0.0;
    const double new_time = program_time();
    delta_time = float(new_time - time);
    time = new_time;
}

void process_inputs(GLFWwindow* window, Camera& camera) {
    static glm::dvec2 mouse_pos;

    glm::dvec2 new_mouse_pos;
    glfwGetCursorPos(window, &new_mouse_pos.x, &new_mouse_pos.y);

    {
        glm::vec3 movement = {};
        if(glfwGetKey(window, 'W') == GLFW_PRESS) {
            movement += camera.forward();
        }
        if(glfwGetKey(window, 'S') == GLFW_PRESS) {
            movement -= camera.forward();
        }
        if(glfwGetKey(window, 'D') == GLFW_PRESS) {
            movement += camera.right();
        }
        if(glfwGetKey(window, 'A') == GLFW_PRESS) {
            movement -= camera.right();
        }

        float speed = 10.0f;
        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            speed *= 10.0f;
        }

        if(movement.length() > 0.0f) {
            const glm::vec3 new_pos = camera.position() + movement * delta_time * speed;
            camera.set_view(glm::lookAt(new_pos, new_pos + camera.forward(), camera.up()));
        }
    }

    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        const glm::vec2 delta = glm::vec2(mouse_pos - new_mouse_pos) * 0.01f;
        if(delta.length() > 0.0f) {
            glm::mat4 rot = glm::rotate(glm::mat4(1.0f), delta.x, glm::vec3(0.0f, 1.0f, 0.0f));
            rot = glm::rotate(rot, delta.y, camera.right());
            camera.set_view(glm::lookAt(camera.position(), camera.position() + (glm::mat3(rot) * camera.forward()), (glm::mat3(rot) * camera.up())));
        }
    }

    {
        int width = 0;
        int height = 0;
        glfwGetWindowSize(window, &width, &height);
        camera.set_ratio(float(width) / float(height));
    }

    mouse_pos = new_mouse_pos;
}

void gui(ImGuiRenderer& imgui) {
    imgui.start();

    DEFER(imgui.finish());

    // ImGui::ShowDemoWindow();

    bool open_scene_popup = false;
    if(ImGui::BeginMainMenuBar()) {
        if(ImGui::BeginMenu("File")) {
            if(ImGui::MenuItem("Open Scene")) {
                open_scene_popup = true;
            }
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Exposure")) {
            ImGui::DragFloat("Exposure", &exposure, 0.25f, 0.01f, 100.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
            if(exposure != 1.0f && ImGui::Button("Reset")) {
                exposure = 1.0f;
            }
            ImGui::EndMenu();
        }

        if(scene && ImGui::BeginMenu("Scene Info")) {
            ImGui::Text("%u objects", u32(scene->objects().size()));
            ImGui::Text("%u point lights", u32(scene->point_lights().size()));
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Debug")) 
        {
            static const char* current_item = "None";
            if (ImGui::BeginCombo("Debug mode", current_item))
            {
                bool none = (!g_buffer_isdepth && !g_buffer_isalbedo && !g_buffer_isnormal);
                if (ImGui::Selectable("None", none))
                {
                    g_buffer_isalbedo = false;
                    g_buffer_isdepth = false;
                    g_buffer_isnormal = false;
                    current_item = "None";
                }
                if (none)
                        ImGui::SetItemDefaultFocus();

                if (ImGui::Selectable("Albedo", g_buffer_isalbedo))
                {
                    g_buffer_isalbedo = true;
                    g_buffer_isdepth = false;
                    g_buffer_isnormal = false;
                    current_item = "Albedo";
                }
                if (g_buffer_isalbedo)
                        ImGui::SetItemDefaultFocus();

                if (ImGui::Selectable("Normals", g_buffer_isnormal))
                {
                    g_buffer_isalbedo = false;
                    g_buffer_isdepth = false;
                    g_buffer_isnormal = true;
                    current_item = "Normals";
                }
                if (g_buffer_isnormal)
                        ImGui::SetItemDefaultFocus();
                
                if (ImGui::Selectable("Depth", g_buffer_isdepth))
                {
                    g_buffer_isalbedo = false;
                    g_buffer_isdepth = true;
                    g_buffer_isnormal = false;
                    current_item = "Depth";
                }
                if (g_buffer_isdepth)
                        ImGui::SetItemDefaultFocus();

                ImGui::EndCombo();
            }
            ImGui::EndMenu();
        }

    
        ImGui::Separator();
        ImGui::TextUnformatted(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

        ImGui::Separator();
        ImGui::Text("%.2f ms", delta_time * 1000.0f);

#ifdef OM3D_DEBUG
        ImGui::PushStyleColor(ImGuiCol_Text, 0xFF0000FF);
        ImGui::TextUnformatted("(DEBUG)");
        ImGui::PopStyleColor();
#endif

        ImGui::EndMainMenuBar();
    }

    if(open_scene_popup) {
        ImGui::OpenPopup("###openscenepopup");

        scene_files.clear();
        for(auto&& entry : std::filesystem::directory_iterator(data_path)) {
            if(entry.status().type() == std::filesystem::file_type::regular) {
                const auto ext = entry.path().extension();
                if(ext == ".gltf" || ext == ".glb") {
                    scene_files.emplace_back(entry.path().string());
                }
            }
        }
    }

    if(ImGui::BeginPopup("###openscenepopup", ImGuiWindowFlags_AlwaysAutoResize)) {
        auto load_scene = [](const std::string path) {
            auto result = Scene::from_gltf(path);
            if(!result.is_ok) {
                std::cerr << "Unable to load scene (" << path << ")" << std::endl;
            } else {
                scene = std::move(result.value);
            }
            ImGui::CloseCurrentPopup();
        };

        char buffer[1024] = {};
        if(ImGui::InputText("Load scene", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
            load_scene(buffer);
        }

        if(!scene_files.empty()) {
            for(const std::string& p : scene_files) {
                const auto abs = std::filesystem::absolute(p).string();
                if(ImGui::MenuItem(abs.c_str())) {
                    load_scene(p);
                    break;
                }
            }
        }

        ImGui::EndPopup();
    }
}




std::unique_ptr<Scene> create_default_scene() {
    auto scene = std::make_unique<Scene>();

    // Load default cube model
    auto result = Scene::from_gltf(std::string(data_path) + "forest.glb");
    ALWAYS_ASSERT(result.is_ok, "Unable to load default scene");
    scene = std::move(result.value);

    scene->set_sun(glm::vec3(0.2f, 1.0f, 0.1f), glm::vec3(1.0f));

    // Add lights
    {
        PointLight light;
        light.set_position(glm::vec3(1.0f, 2.0f, 4.0f));
        light.set_color(glm::vec3(0.0f, 50.0f, 0.0f));
        light.set_radius(100.0f);
        scene->add_light(std::move(light));
    }
    {
        PointLight light;
        light.set_position(glm::vec3(1.0f, 2.0f, -4.0f));
        light.set_color(glm::vec3(50.0f, 0.0f, 0.0f));
        light.set_radius(50.0f);
        scene->add_light(std::move(light));
    }

    return scene;
}

struct RendererState {
    static RendererState create(glm::uvec2 size) {
        RendererState state;

        state.size = size;

        if(state.size.x > 0 && state.size.y > 0) {
            //state.lit_hdr_texture = Texture(size, ImageFormat::RGBA16_FLOAT);

            state.g_buffer_texture = Texture(size, ImageFormat::RGBA8_UNORM);
            state.tone_mapped_texture = Texture(size, ImageFormat::RGBA8_UNORM);

            //state.main_framebuffer = Framebuffer(&state.depth_texture, std::array{&state.lit_hdr_texture});
            state.tone_map_framebuffer = Framebuffer(nullptr, std::array{&state.tone_mapped_texture});

            state.depth_texture = Texture(size, ImageFormat::Depth32_FLOAT);
            state.g_buffer_albedo = Texture(size, ImageFormat::RGBA8_sRGB);
            state.g_buffer_normal = Texture(size, ImageFormat::RGBA8_UNORM);

            state.g_buffer = Framebuffer(&state.depth_texture, std::array{&state.g_buffer_albedo, &state.g_buffer_normal});
            state.g_buffer_debug = Framebuffer(nullptr, std::array{&state.g_buffer_texture});

            state.lights_texture = Texture(size, ImageFormat::RGBA8_UNORM);
            state.sun_light = Framebuffer(nullptr, std::array{&state.lights_texture});
        }

        return state;
    }

    glm::uvec2 size = {};

    //Texture lit_hdr_texture;

    Texture tone_mapped_texture;
    Texture g_buffer_texture;

    Texture depth_texture;
    Texture g_buffer_albedo;
    Texture g_buffer_normal;

    //Framebuffer main_framebuffer;
    Framebuffer tone_map_framebuffer;

    Framebuffer g_buffer;
    Framebuffer g_buffer_debug;

    Framebuffer sun_light;
    Texture lights_texture;
};





int main(int argc, char** argv) {
    DEBUG_ASSERT([] { std::cout << "Debug asserts enabled" << std::endl; return true; }());

    parse_args(argc, argv);

    glfw_check(glfwInit());
    DEFER(glfwTerminate());

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    GLFWwindow* window = glfwCreateWindow(1600, 900, "TP window", nullptr, nullptr);
    glfw_check(window);
    DEFER(glfwDestroyWindow(window));

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    init_graphics();

    ImGuiRenderer imgui(window);

    scene = create_default_scene();

    auto tonemap_program = Program::from_files("tonemap.frag", "screen.vert");
    auto g_buffer_program = Program::from_files("debug_g_buffer.frag", "screen.vert");
    auto lights_program = Program::from_files("sun.frag", "screen.vert");

    RendererState renderer;

    for(;;) {

        glfwPollEvents();
        if(glfwWindowShouldClose(window) || glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            break;
        }

        {
            int width = 0;
            int height = 0;
            glfwGetWindowSize(window, &width, &height);

            if(renderer.size != glm::uvec2(width, height)) {
                renderer = RendererState::create(glm::uvec2(width, height));
            }
        }

        update_delta_time();

        if(const auto& io = ImGui::GetIO(); !io.WantCaptureMouse && !io.WantCaptureKeyboard) {
            process_inputs(window, scene->camera());
        }

        // Render the scene
        {
            //renderer.main_framebuffer.bind();
            glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "g_buffer");
            renderer.g_buffer.bind();
            scene->render();
            glPopDebugGroup();
        }
        glDisable(GL_CULL_FACE);

        if (!g_buffer_isdepth && !g_buffer_isalbedo && !g_buffer_isnormal)
        {
            {
                glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, "1 2 3 Soleil");
                renderer.sun_light.bind(false);
                lights_program->bind();
                renderer.g_buffer_albedo.bind(0);
                renderer.g_buffer_normal.bind(1);
                scene->renderLights();
                glPopDebugGroup();

                glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 2, -1, "Tone Map");
                glPopDebugGroup();
                renderer.tone_map_framebuffer.bind();
                tonemap_program->bind();
                tonemap_program->set_uniform(HASH("exposure"), exposure);
                renderer.lights_texture.bind(0);
                glDrawArrays(GL_TRIANGLES, 0, 3);
            }
        }
        else
        {
            renderer.g_buffer_debug.bind();
            g_buffer_program->bind();
            g_buffer_program->set_uniform(HASH("isdepth"), g_buffer_isdepth);
            g_buffer_program->set_uniform(HASH("isalbedo"), g_buffer_isalbedo);
            g_buffer_program->set_uniform(HASH("isnormal"), g_buffer_isnormal);
            renderer.depth_texture.bind(0);
            renderer.g_buffer_albedo.bind(1);
            renderer.g_buffer_normal.bind(2);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

        // Blit tonemap result to screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        renderer.tone_map_framebuffer.blit();
        if (!g_buffer_isdepth && !g_buffer_isalbedo && !g_buffer_isnormal)
        {
            renderer.sun_light.blit();
            renderer.tone_map_framebuffer.blit();
        }
        else
            renderer.g_buffer_debug.blit();

        gui(imgui);

        glfwSwapBuffers(window);
    }

    scene = nullptr; // destroy scene and child OpenGL objects
}
