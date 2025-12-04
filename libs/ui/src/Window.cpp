#include <ontoflow/core/Logger.hpp>
#include <ontoflow/ui/Window.hpp>

#include "GLFW/glfw3.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

// Global texture for preventing shader warnings when no texture is bound
static GLuint g_DefaultTexture = 0;

/**
 * rief Creates a default 1x1 white texture.
 * This texture can be used to prevent shader warnings when a shader expects
 * a texture but none is explicitly bound by the application.
 */
static void CreateDefaultTexture() {
    unsigned char whitePixel[4] = {255, 255, 255, 255};  // RGBA white

    glGenTextures(1, &g_DefaultTexture);
    glBindTexture(GL_TEXTURE_2D, g_DefaultTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    LOG(Info) << "Created default 1x1 white texture (ID: " << g_DefaultTexture << ").";
}

/**
 * rief Destroys the global default texture.
 */
static void DestroyDefaultTexture() {
    if (g_DefaultTexture) {
        glDeleteTextures(1, &g_DefaultTexture);
        g_DefaultTexture = 0;
        LOG(Info) << "Destroyed default texture.";
    }
}

namespace of::ui {

/**
 * rief Creates and initializes a GLFW window and its OpenGL context.
 * 
 * \param ci Configuration information for the window.
 * \return True if the window was created successfully, false otherwise.
 */
bool Window::Create(const CreateInfo& ci) {
    if (!glfwInit()) {
        LOG(Error) << "GLFW init failed.";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_SAMPLES, ci.msaa);

    m_Window = glfwCreateWindow(ci.width, ci.height, ci.title.c_str(), nullptr, nullptr);
    if (!m_Window) {
        LOG(Error) << "Failed to create GLFW window.";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_Window);
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        LOG(Error) << "Failed to init GLAD.";
        glfwTerminate();
        return false;
    }

    CreateDefaultTexture();

    glfwSwapInterval(1);  // VSync
    InitCallbacks();

    // ImGui Setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

    ImGui::StyleColorsDark();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(m_Window, false);
    ImGui_ImplOpenGL3_Init("#version 410");

    // Get initial framebuffer size and set viewport
    glfwGetFramebufferSize(m_Window, &m_FramebufferWidth, &m_FramebufferHeight);
    glViewport(0, 0, m_FramebufferWidth, m_FramebufferHeight);

    LOG(Info) << "Window created: " << ci.title << " (" << ci.width << "x" << ci.height << ", MSAA: " << ci.msaa << ")";
    return true;
}

/**
 * \brief Destroys the GLFW window and terminates GLFW.
 * This should be called once when the application shuts down.
 */
void Window::Destroy() {
    // ImGui Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    DestroyDefaultTexture(); // Clean up the global default texture
    if (m_Window) {
        glfwDestroyWindow(m_Window);
        m_Window = nullptr;
    }
    glfwTerminate();
    LOG(Info) << "Window destroyed and GLFW terminated.";
}

void Window::BeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Window::EndFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

/**
 * \brief Requests the window to close.
 */
void Window::Close() {
    if (m_Window) {
        glfwSetWindowShouldClose(m_Window, true);
    }
}

/**
 * \brief Polls for and processes pending GLFW events.
 * \return True if the window should remain open, false if it should close.
 */
bool Window::PollEvents() {
    if (!m_Window || glfwWindowShouldClose(m_Window))
        return false;
    glfwPollEvents();
    return true;
}

/**
 * \brief Swaps the front and back buffers of the window, presenting the rendered frame.
 */
void Window::SwapBuffers() {
    if (m_Window) {
        glfwSwapBuffers(m_Window);
    }
}

/**
 * \brief Initializes GLFW callbacks for the window.
 * This method sets up static GLFW callback functions that dispatch
 * to the appropriate std::function members using the user pointer.
 */
void Window::InitCallbacks() {
    glfwSetWindowUserPointer(m_Window, this);

    // Key event callback
    glfwSetKeyCallback(m_Window, [](GLFWwindow* w, int key, int scancode, int action, int mods) {
        ImGui_ImplGlfw_KeyCallback(w, key, scancode, action, mods); // Pass to ImGui first
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureKeyboard) { // If ImGui consumed the keyboard event, don't pass it further
            return;
        }

        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
        if (!self || !self->m_KeyPressedCallback) // Check if callback is set
            return;
        self->m_KeyPressedCallback(key, scancode, action, mods); // Pass all GLFW key data
    });

    // Mouse button event callback
    glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* w, int button, int action, int mods) {
        ImGui_ImplGlfw_MouseButtonCallback(w, button, action, mods); // Pass to ImGui first
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse) { // If ImGui consumed the mouse event, don't pass it further
            return;
        }

        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
        if (!self || !self->m_MouseButtonCallback) // Check if callback is set
            return;
        self->m_MouseButtonCallback(button, action, mods);
    });

    // Scroll event callback
    glfwSetScrollCallback(m_Window, [](GLFWwindow* w, double xoff, double yoff) {
        ImGui_ImplGlfw_ScrollCallback(w, xoff, yoff); // Pass to ImGui first
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse) { // If ImGui consumed the mouse event, don't pass it further
            return;
        }

        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
        if (!self || !self->m_ScrollCallback) // Check if callback is set
            return;
        self->m_ScrollCallback(xoff, yoff);
    });

    // Cursor position callback
    glfwSetCursorPosCallback(m_Window, [](GLFWwindow* w, double x, double y) {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
        if (!self || !self->m_MouseMoveCallback) // Check if callback is set
            return;
        self->m_MouseMoveCallback(x, y);
    });

    // Framebuffer size callback
    glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* w, int width, int height) {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
        if (!self)
            return;
        self->m_FramebufferWidth = width; // Update internal dimensions
        self->m_FramebufferHeight = height;
        glViewport(0, 0, width, height); // Update OpenGL viewport
        if (self->m_WindowSizeCallback) { // Call user-defined callback if set
            self->m_WindowSizeCallback(width, height);
        }
    });
}

/**
 * \brief Returns the aspect ratio of the window's framebuffer.
 * \return The width divided by the height.
 */
float Window::Aspect() const {
    if (m_FramebufferHeight == 0) return 1.0f; // Prevent division by zero
    return static_cast<float>(m_FramebufferWidth) / static_cast<float>(m_FramebufferHeight);
}

/**
 * \brief Sets a callback for mouse button press/release events.
 * The callback receives GLFW button, action (press/release), and modifiers.
 * \param cb The callback function.
 */
void Window::SetMouseButtonCallback(std::function<void(int button, int action, int mods)> cb) {
    m_MouseButtonCallback = std::move(cb);
}

/**
 * \brief Sets a callback for mouse movement events.
 * The callback receives the new mouse cursor position (x, y).
 * \param cb The callback function.
 */
void Window::SetMouseMoveCallback(std::function<void(double x, double y)> cb) {
    m_MouseMoveCallback = std::move(cb);
}

/**
 * \brief Sets a callback for key press/release events.
 * The callback receives GLFW key code, scancode, action (press/release/repeat), and modifiers.
 * \param cb The callback function.
 */
void Window::SetKeyPressedCallback(std::function<void(int key, int scancode, int action, int mods)> cb) {
    m_KeyPressedCallback = std::move(cb);
}

/**
 * \brief Sets a callback for scroll wheel events.
 * The callback receives the scroll offset (dx, dy).
 * \param cb The callback function.
 */
void Window::SetScrollCallback(std::function<void(double dx, double dy)> cb) {
    m_ScrollCallback = std::move(cb);
}

/**
 * \brief Sets a callback for window framebuffer size changes.
 * The callback receives the new width and height of the framebuffer.
 * \param cb The callback function.
 */
void Window::SetWindowSizeCallback(std::function<void(int w, int h)> cb) {
    m_WindowSizeCallback = std::move(cb);
}
}  // namespace of::ui
