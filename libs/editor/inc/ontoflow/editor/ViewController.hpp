#pragma once

#include <memory>
#include <ontoflow/editor/ICamera.hpp>
#include <ontoflow/editor/InputEvent.hpp>

namespace of::editor {

/**
 * \brief Defines the high-level viewing modes for the ViewController.
 */
enum class ViewMode {
    View3D,   ///< Standard 3D perspective view.
    Sketch2D  ///< 2D orthographic sketching view.
};

/**
 * \brief Manages camera selection and dispatches input events to the active camera.
 *
 * This class orchestrates the interaction between editor input and camera control,
 * allowing for switching between 2D and 3D viewing modes and routing mouse/keyboard
 * events to the currently active camera.
 */
class ViewController {
   public:
    /**
     * \brief Constructs a ViewController.
     * Initializes with a default 3D view mode and a default viewport size.
     */
    ViewController();

    /**
     * \brief Sets the viewport dimensions for all managed cameras.
     * \param width Width of the viewport in pixels.
     * \param height Height of the viewport in pixels.
     */
    void SetViewportSize(int width, int height);

    /**
     * \brief Sets the 3D camera instance.
     * \param cam A shared pointer to the ICamera implementation for 3D view.
     */
    void SetCamera3D(std::shared_ptr<ICamera> cam);

    /**
     * \brief Sets the 2D camera instance.
     * \param cam A shared pointer to the ICamera implementation for 2D sketch view.
     */
    void SetCamera2D(std::shared_ptr<ICamera> cam);

    /**
     * \brief Returns a pointer to the currently active camera.
     * \return A pointer to the ICamera, or nullptr if no camera is active.
     */
    ICamera* GetActiveCamera() const;

    /**
     * \brief Returns the current viewing mode.
     * \return The current ViewMode.
     */
    ViewMode GetMode() const {
        return m_Mode;
    }

    /**
     * \brief Switches the viewing mode (e.g., between 3D and 2D sketch).
     * \param mode The target ViewMode.
     */
    void SwitchMode(ViewMode mode);

    /**
     * \brief Handles an input event, dispatching it to the active camera.
     * \param ev The input event to process.
     */
    void OnInput(const InputEvent& ev);

    /**
     * \brief Updates the active camera's state.
     * \param dt Time delta since the last update.
     */
    void Update(double dt);

   private:
    ViewMode m_Mode = ViewMode::View3D;  ///< Current viewing mode.
    int m_Width = 1280;                  ///< Current viewport width.
    int m_Height = 720;                  ///< Current viewport height.

    std::shared_ptr<ICamera> m_Cam3D;  ///< Shared pointer to the 3D camera.
    std::shared_ptr<ICamera> m_Cam2D;  ///< Shared pointer to the 2D camera.

    bool m_LMB = false;  ///< State of the Left Mouse Button.
    bool m_RMB = false;  ///< State of the Right Mouse Button.
    // m_LastX, m_LastY removed as they were unused (handled by camera's OnMouseStart)
};

}  // namespace of::editor
