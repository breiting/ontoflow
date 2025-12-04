#include <memory>
#include <ontoflow/core/Logger.hpp>
#include <ontoflow/domain/Components.hpp>
#include <ontoflow/domain/Entity.hpp>
#include <ontoflow/domain/GeometrySystem.hpp>
#include <ontoflow/domain/Registry.hpp>
#include <ontoflow/editor/Editor.hpp>
#include <ontoflow/occt/OCCTBackend.hpp>
#include <ontoflow/ui/Window.hpp>
#include <ontoflow/vis/Camera2D.hpp>
#include <ontoflow/vis/Camera3D.hpp>
#include <ontoflow/vis/OpenGLRenderer.hpp>
#include <ontoflow/vis/RenderingSystem.hpp>

// GLFW includes are usually handled by Window.hpp, but we need keys here
#include <GLFW/glfw3.h>

using namespace of::domain;
using namespace of::occt;
using namespace of::editor;
using namespace of::ui;
using namespace of::vis;

constexpr int WINDOW_WIDTH = 1600;
constexpr int WINDOW_HEIGHT = 900;
constexpr const char* APP_NAME = "OntoFlow | Dataflow Engine";

// Helper: Convert GLFW Input to OntoFlow Input Event
static KeyEvent MakeKeyEventFromGLFW(int key, int /*action*/, int mods) {
    using namespace of;
    KeyEvent ev{};
    ev.pressed = true;  // simplifying for press events
    ev.ctrl = (mods & GLFW_MOD_CONTROL) != 0;
    ev.alt = (mods & GLFW_MOD_ALT) != 0;
    ev.shift = (mods & GLFW_MOD_SHIFT) != 0;

    switch (key) {
        case GLFW_KEY_ESCAPE:
            ev.code = KeyCode::Escape;
            break;
        case GLFW_KEY_ENTER:
            ev.code = KeyCode::Enter;
            break;
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

int main(int argc, char* argv[]) {
    LOG(Info) << "Starting " << APP_NAME << "...";

    // ------------------------------------------------------------
    // 1. CORE SYSTEM INITIALIZATION
    // ------------------------------------------------------------
    Registry registry;
    OCCTBackend backend;  // Stateless Geometry Kernel
    GeometrySystem geom(registry, backend);

    // ------------------------------------------------------------
    // 2. EDITOR SETUP (The Brain)
    // ------------------------------------------------------------
    Editor editor(registry, geom);

    // CRITICAL: Load the initial "Value -> Box" Graph
    LOG(Info) << "Initializing Demo Graph...";
    editor.InitializeDemoGraph();

    // ------------------------------------------------------------
    // 3. VISUALIZATION SETUP (The Eyes)
    // ------------------------------------------------------------
    Window window;
    if (!window.Create({WINDOW_WIDTH, WINDOW_HEIGHT, APP_NAME})) {
        LOG(Error) << "Failed to create window";
        return -1;
    }

    // Renderer (OpenGL)
    auto renderer = std::make_unique<OpenGLRenderer>();
    RenderingSystem renderingSystem(std::move(renderer));
    renderingSystem.Init(registry);
    renderingSystem.SetShowAxis(true);
    renderingSystem.SetViewportSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    // Cameras
    auto cam3D = std::make_shared<Camera3D>();
    // Position camera to look at the box (approximate)
    // Assuming Camera3D has a sensible default or SetPosition method

    editor.SetCamera3D(cam3D);
    editor.SetViewportSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    // ------------------------------------------------------------
    // 4. INPUT MAPPING
    // ------------------------------------------------------------
    window.SetKeyPressedCallback([&](int key, int scancode, int action, int mods) {
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            InputEvent ev;
            ev.type = InputEventType::Key;
            ev.data = MakeKeyEventFromGLFW(key, action, mods);
            editor.OnInput(ev);
        }
    });

    window.SetMouseButtonCallback([&](int btn, int act, int mods) {
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

    // ------------------------------------------------------------
    // 5. MAIN LOOP
    // ------------------------------------------------------------
    double lastTime = glfwGetTime();

    while (window.PollEvents()) {
        double currentTime = glfwGetTime();
        double dt = currentTime - lastTime;
        lastTime = currentTime;

        // A. Logic Update (Graph Evaluation happens here if dirty)
        editor.Update(dt);

        // B. Sync Geometry (MeshComponent updates if Graph changed)
        renderingSystem.Update(registry);

        // C. Render 3D Scene (Background)
        auto* cam = editor.GetActiveCamera();
        if (cam) {
            renderingSystem.Render(cam);
        }

        // D. Render UI (Graph Editor on top)
        window.BeginFrame();  // Starts ImGui Frame
        editor.DrawUI();      // Draws ImNodes
        window.EndFrame();    // Ends ImGui Frame & Renders DrawData

        window.SwapBuffers();
    }

    LOG(Info) << "Shutting down OntoFlow.";
    window.Destroy();
    return 0;
}
