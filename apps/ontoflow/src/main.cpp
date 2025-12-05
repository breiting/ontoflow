#include <ontoflow/core/Logger.hpp>
#include <ontoflow/domain/GeometrySystem.hpp>
#include <ontoflow/domain/Registry.hpp>
#include <ontoflow/editor/Editor.hpp>
#include <ontoflow/nodes/StandardLibrary.hpp>
#include <ontoflow/render/OpenGLRenderer.hpp>
#include <ontoflow/render/RenderingSystem.hpp>
#include <ontoflow/ui/Window.hpp>

#include "ontoflow/occt/OCCTBackend.hpp"
#include "ontoflow/render/Camera2D.hpp"
#include "ontoflow/render/Camera3D.hpp"

using namespace of;
using namespace of::editor;
using namespace of::render;
using namespace of::nodes;

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

int main() {
    // ----------------------------------------------------
    // 1. Create Window
    // ----------------------------------------------------
    ui::Window window;

    ui::Window::CreateInfo ci;
    ci.width = 1600;
    ci.height = 1000;
    ci.title = "OntoFlow Editor";

    if (!window.Create(ci)) {
        LOG(Error) << "Failed to create OntoFlow window.";
        return 1;
    }

    LOG(Info) << "OntoFlow Window created.";

    // ----------------------------------------------------
    // 2. OntoFlow Core Systems
    // ----------------------------------------------------
    domain::Registry registry;
    occt::OCCTBackend backend;
    domain::GeometrySystem geometrySystem(registry, backend);

    StandardLibrary::RegisterAll(backend);

    // RenderingSystem (after Window initialization)
    auto renderer = std::make_unique<OpenGLRenderer>();
    RenderingSystem renderingSystem(std::move(renderer));
    renderingSystem.Init(registry);
    renderingSystem.SetShowAxis(true);

    auto cam2D = std::make_shared<Camera2D>();
    auto cam3D = std::make_shared<Camera3D>();

    editor::Editor editor(registry, geometrySystem, &renderingSystem);

    editor.SetCamera2D(cam2D);
    editor.SetCamera3D(cam3D);

    // Demo nodes
    // editor.InitializeDemoGraph();

    // ----------------------------------------------------
    // 3. Connect window input → editor input
    // ----------------------------------------------------
    window.SetKeyPressedCallback([&](int key, int /*scancode*/, int action, int /*mods*/) {
        InputEvent ev;
        ev.type = InputEventType::Key;
        ev.data = MakeKeyEventFromGLFW(key, action, 0);
        editor.OnInput(ev);
    });

    window.SetMouseButtonCallback([&](int button, int action, int /*mods*/) {
        double x, y;
        glfwGetCursorPos(window.GetNative(), &x, &y);
        InputEvent ev;
        ev.type = InputEventType::MouseButton;
        ev.data = MouseButtonEvent{
            (button == GLFW_MOUSE_BUTTON_LEFT ? MouseButton::Left : MouseButton::Right), action == GLFW_PRESS, {x, y}};
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
    });

    // ----------------------------------------------------
    // 4. Main Application Loop
    // ----------------------------------------------------
    while (window.PollEvents()) {
        // Start ImGui frame
        window.BeginFrame();

        // Draw editor UI
        editor.DrawUI();

        // Update camera + graph
        editor.Update(1.0 / 60.0);

        // Render
        window.EndFrame();
        window.SwapBuffers();
    }

    window.Destroy();
    return 0;
}
