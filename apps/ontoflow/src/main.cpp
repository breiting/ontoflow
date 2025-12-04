#include <CLI/CLI.hpp>
#include <memory>
#include <ontoflow/core/Logger.hpp>
#include <ontoflow/domain/Components.hpp>
#include <ontoflow/domain/Entity.hpp>
#include <ontoflow/domain/GeometrySystem.hpp>
#include <ontoflow/domain/IGeometryBackend.hpp>
#include <ontoflow/domain/Query.hpp>
#include <ontoflow/domain/Registry.hpp>
#include <ontoflow/editor/Editor.hpp>
#include <ontoflow/occt/OCCTBackend.hpp>
#include <ontoflow/occt/STEPImporter.hpp>
#include <ontoflow/ui/Window.hpp>
#include <ontoflow/vis/Camera2D.hpp>
#include <ontoflow/vis/Camera3D.hpp>
#include <ontoflow/vis/Mesh.hpp>
#include <ontoflow/vis/OpenGLRenderer.hpp>
#include <ontoflow/vis/RenderingSystem.hpp>
#include <ontoflow/vis/StlReader.hpp>

#include "CLI/CLI.hpp"

using namespace of::domain;
using namespace of::occt;
using namespace of::editor;
using namespace of::ui;
using namespace of::vis;

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 800;
constexpr const char* APP_NAME = "ontoflow";

static KeyEvent MakeKeyEventFromGLFW(int key, int /*action*/, int mods) {
    using namespace of;
    KeyEvent ev{};
    ev.pressed = true;
    ev.ctrl = (mods & GLFW_MOD_CONTROL) != 0;
    ev.alt = (mods & GLFW_MOD_ALT) != 0;
    ev.shift = (mods & GLFW_MOD_SHIFT) != 0;

    switch (key) {
        case GLFW_KEY_ESCAPE:
            ev.code = KeyCode::Escape;
            break;
        case GLFW_KEY_ENTER:
        case GLFW_KEY_KP_ENTER:
            ev.code = KeyCode::Enter;
            break;
        case GLFW_KEY_SPACE:
            ev.code = KeyCode::Space;
            break;
        default:
            ev.code = KeyCode::Unknown;
            break;
    }

    ev.text = 0;
    if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
        char base = ev.shift ? 'A' : 'a';
        ev.text = static_cast<char>(base + (key - GLFW_KEY_A));
    }
    if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9 && !ev.shift) {
        ev.text = static_cast<char>('0' + (key - GLFW_KEY_0));
    }

    return ev;
}

Entity LoadSTLtoECS(const std::string& file, Registry& ecs) {
    of::domain::Mesh mesh;
    StlReader reader;

    if (!reader.LoadFromFile(file, mesh)) {
        LOG(Error) << "Failed to load STL: " << file << "\n";
        return INVALID_ENTITY;
    }

    Entity e = ecs.CreateEntity();
    ecs.AddComponent<MeshComponent>(e, MeshComponent{mesh});
    ecs.AddComponent<NameComponent>(e, NameComponent{"ImportedSTL"});
    LOG(Info) << "Successfully added entity: " << e;

    return e;
}

/*
Entity CreateTestFace5(Registry& ecs) {
    // LEGACY FUNCTION REMOVED
    return INVALID_ENTITY;
}
*/

int main(int argc, char* argv[]) {
    CLI::App app{"Desc"};
    std::string stlFile;
    app.add_option("--stl", stlFile, "Load STL model");
    std::string stepFile;
    app.add_option("--step", stepFile, "Load STEP model");
    bool loadCube = false;
    app.add_flag("--cube", loadCube, "Load unit cube");
    bool loadFace = false;
    app.add_flag("--face", loadFace, "Load test face");

    CLI11_PARSE(app, argc, argv);

    LOG(Info) << "================================";
    LOG(Info) << APP_NAME;
    LOG(Info) << "================================";

    // ECS + Backend

    Registry registry;
    OCCTBackend backend;
    GeometrySystem geom(registry, backend);

    if (!stlFile.empty()) {
        Entity stl = LoadSTLtoECS(stlFile, registry);
        if (stl == INVALID_ENTITY) {
            LOG(Error) << "Error loading STL file";
            return -1;
        }
    }

    if (!stepFile.empty()) {
        STEPImporter step;
        Entity mesh = step.Load(stepFile, registry);
        if (mesh == INVALID_ENTITY) {
            LOG(Error) << "Error loading STEP file";
            return -1;
        }
    }

    Editor editor(registry, geom);

    // Initialize Demo Graph
    editor.InitializeDemoGraph();

    // Window
    Window window;
    if (!window.Create({WINDOW_WIDTH, WINDOW_HEIGHT, APP_NAME}))
        return -1;

    // RenderingSystem (after Window initialization)
    auto renderer = std::make_unique<OpenGLRenderer>();
    RenderingSystem renderingSystem(std::move(renderer));
    renderingSystem.Init(registry);
    renderingSystem.SetShowAxis(true);

    auto cam2D = std::make_shared<Camera2D>();
    auto cam3D = std::make_shared<Camera3D>();
    editor.SetCamera2D(cam2D);
    editor.SetCamera3D(cam3D);
    editor.SetViewportSize(window.GetWidth(), window.GetHeight());

    // INPUT MAPPING
    window.SetKeyPressedCallback([&](int key, int /*scancode*/, int action, int /*mods*/) {
        LOG(Info) << "GLFW Key Pressed Callback: Key=" << key << ", Action=" << action;
        InputEvent ev;
        ev.type = InputEventType::Key;
        ev.data = MakeKeyEventFromGLFW(key, action, 0);
        editor.OnInput(ev);
    });

    window.SetMouseButtonCallback([&](int btn, int act, int /*mods*/) {
        double x, y;
        glfwGetCursorPos(window.GetNative(), &x, &y);
        InputEvent ev;
        ev.type = InputEventType::MouseButton;
        ev.data = MouseButtonEvent{
            (btn == GLFW_MOUSE_BUTTON_LEFT ? MouseButton::Left : MouseButton::Right), act == GLFW_PRESS, {x, y}};
        editor.OnInput(ev);
    });

    window.SetMouseMoveCallback([&](double x, double y) {
        InputEvent ev;
        ev.type = InputEventType::MouseMove;
        MouseMoveEvent mm{};
        mm.position = {x, y};
        ev.data = mm;
        editor.OnInput(ev);
    });

    window.SetScrollCallback([&](double dx, double dy) {
        InputEvent ev;
        ev.type = InputEventType::Scroll;
        ScrollEvent sc{};
        sc.offset = {dx, dy};
        ev.data = sc;
        editor.OnInput(ev);
    });

    window.SetWindowSizeCallback([&](int w, int h) {
        editor.SetViewportSize(w, h);
        renderingSystem.SetViewportSize(w, h);
    });

    auto lt = static_cast<float>(glfwGetTime());
    while (window.PollEvents()) {
        float ct = static_cast<float>(glfwGetTime());
        float dt = ct - lt;
        lt = ct;

        // UPDATE
        editor.Update(dt);
        renderingSystem.Update(registry);

        // RENDER
        auto* cam = editor.GetActiveCamera();
        assert(cam);
        renderingSystem.Render(cam);

        // UI RENDER
        window.BeginFrame();
        editor.DrawUI();
        window.EndFrame();

        window.SwapBuffers();
    }

    window.Destroy();
    return 0;
}
