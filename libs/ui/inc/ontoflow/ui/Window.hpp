#pragma once

// clang-format off
#include <glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <functional>
#include <string>

namespace of::ui {

/**
 * \brief Encapsulates a GLFW window, handling creation, input, and OpenGL context management.
 */
class Window {
   public:
    /**
     * \brief Configuration structure for creating a new window.
     */
    struct CreateInfo {
        int width = 1280;    ///< Initial width of the window.
        int height = 720;    ///< Initial height of the window.
        std::string title = "OntoFlow"; ///< Title displayed in the window's title bar.
        int msaa = 4;        ///< Multi-sample anti-aliasing level.
    };

    /**
     * \brief Creates and initializes a GLFW window and its OpenGL context.
     * \param ci Configuration information for the window.
     * \return True if the window was created successfully, false otherwise.
     */
    bool Create(const CreateInfo &ci);

    /**
     * \brief Requests the window to close.
     */
    void Close();

    /**
     * \brief Destroys the GLFW window and terminates GLFW.
     * This should be called once when the application shuts down.
     */
    void Destroy();

    /**
     * \brief Polls for and processes pending GLFW events.
     * \return True if the window should remain open, false if it should close.
     */
    bool PollEvents();

    /**
     * \brief Swaps the front and back buffers of the window, presenting the rendered frame.
     */
    void SwapBuffers();

    /**
     * \brief Starts a new ImGui frame.
     * Call this before any ImGui calls.
     */
    void BeginFrame();

    /**
     * \brief Ends the ImGui frame and renders it.
     * Call this after all ImGui calls and before SwapBuffers.
     */
    void EndFrame();

    /**
     * \brief Returns the aspect ratio of the window's framebuffer.
     * \return The width divided by the height.
     */
    float Aspect() const;

    /**
     * \brief Returns the current width of the window's framebuffer.
     * \return The framebuffer width in pixels.
     */
    int GetWidth() const {
        return m_FramebufferWidth;
    }

    /**
     * \brief Returns the current height of the window's framebuffer.
     * \return The framebuffer height in pixels.
     */
    int GetHeight() const {
        return m_FramebufferHeight;
    }

    /**
     * \brief Sets a callback for key press/release events.
     * The callback receives GLFW key code, scancode, action (press/release/repeat), and modifiers.
     * \param cb The callback function.
     */
    void SetKeyPressedCallback(std::function<void(int key, int scancode, int action, int mods)> cb);
    
    /**
     * \brief Sets a callback for mouse button press/release events.
     * The callback receives GLFW button, action (press/release), and modifiers.
     * \param cb The callback function.
     */
    void SetMouseButtonCallback(std::function<void(int button, int action, int mods)> cb);

    /**
     * \brief Sets a callback for mouse movement events.
     * The callback receives the new mouse cursor position (x, y).
     * \param cb The callback function.
     */
    void SetMouseMoveCallback(std::function<void(double x, double y)> cb);

    /**
     * \brief Sets a callback for scroll wheel events.
     * The callback receives the scroll offset (dx, dy).
     * \param cb The callback function.
     */
    void SetScrollCallback(std::function<void(double dx, double dy)> cb);

    /**
     * \brief Sets a callback for window framebuffer size changes.
     * The callback receives the new width and height of the framebuffer.
     * \param cb The callback function.
     */
    void SetWindowSizeCallback(std::function<void(int w, int h)> cb);

    /**
     * \brief Returns the native GLFWwindow pointer.
     * \return A pointer to the underlying GLFWwindow object.
     */
    GLFWwindow *GetNative() {
        return m_Window;
    }

   private:
    /**
     * \brief Initializes GLFW callbacks for the window.
     * This method sets up static GLFW callback functions that dispatch
     * to the appropriate std::function members using the user pointer.
     */
    void InitCallbacks();

    GLFWwindow *m_Window = nullptr; ///< Pointer to the native GLFWwindow object.
    int m_FramebufferWidth = 0;   ///< Current width of the framebuffer.
    int m_FramebufferHeight = 0;  ///< Current height of the framebuffer.

    std::function<void(int key, int scancode, int action, int mods)> m_KeyPressedCallback; ///< Callback for key events.
    std::function<void(int button, int action, int mods)> m_MouseButtonCallback; ///< Callback for mouse button events.
    std::function<void(double x, double y)> m_MouseMoveCallback; ///< Callback for mouse move events.
    std::function<void(double dx, double dy)> m_ScrollCallback; ///< Callback for scroll events.
    std::function<void(int w, int h)> m_WindowSizeCallback; ///< Callback for window size events.
};

}  // namespace of::ui
