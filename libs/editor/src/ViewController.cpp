#include <ontoflow/core/Logger.hpp>
#include <ontoflow/editor/ViewController.hpp>

namespace of::editor {

/**
 * \brief Constructs a ViewController.
 * Initializes with a default 3D view mode and a default viewport size.
 */
ViewController::ViewController() {
}

/**
 * \brief Sets the viewport dimensions for all managed cameras.
 * \param width Width of the viewport in pixels.
 * \param height Height of the viewport in pixels.
 */
void ViewController::SetViewportSize(int width, int height) {
    m_Width = width;
    m_Height = height;

    if (m_Cam2D) {
        m_Cam2D->SetViewport(width, height);
    }
    if (m_Cam3D) {
        m_Cam3D->SetViewport(width, height);
    }
}

/**
 * \brief Sets the 3D camera instance.
 * \param cam A shared pointer to the ICamera implementation for 3D view.
 */
void ViewController::SetCamera3D(std::shared_ptr<ICamera> cam) {
    m_Cam3D = std::move(cam);
}
/**
 * \brief Sets the 2D camera instance.
 * \param cam A shared pointer to the ICamera implementation for 2D sketch view.
 */
void ViewController::SetCamera2D(std::shared_ptr<ICamera> cam) {
    m_Cam2D = std::move(cam);
}

/**
 * \brief Returns a pointer to the currently active camera.
 * \return A pointer to the ICamera, or nullptr if no camera is active.
 */
ICamera* ViewController::GetActiveCamera() const {
    if (m_Mode == ViewMode::View3D)
        return m_Cam3D.get();
    return m_Cam2D.get();
}

/**
 * \brief Switches the viewing mode (e.g., between 3D and 2D sketch).
 * \param mode The target ViewMode.
 */
void ViewController::SwitchMode(ViewMode mode) {
    if (m_Mode == mode) return; // No change needed
    m_Mode = mode;
    // Potentially trigger OnEnter/OnExit for cameras, or update viewport.
    // Cameras are automatically updated through Update() call.
}

/**
 * \brief Handles an input event, dispatching it to the active camera.
 * \param ev The input event to process.
 */
void ViewController::OnInput(const InputEvent& ev) {
    // Handle global mode switching shortcuts (e.g., '1' for 3D, '2' for 2D)
    if (ev.type == InputEventType::Key) {
        const auto& key = std::get<KeyEvent>(ev.data);
        if (key.pressed) { // Only on key press
            if (key.text == '1') {
                LOG(Info) << "ViewController: Switching to 3D ViewMode.";
                SwitchMode(ViewMode::View3D);
                if (m_Cam3D) m_Cam3D->OnMouseStart(); // Reset mouse tracking for new camera
                return;
            }
            if (key.text == '2') {
                LOG(Info) << "ViewController: Switching to 2D Sketch2D ViewMode.";
                SwitchMode(ViewMode::Sketch2D);
                if (m_Cam2D) m_Cam2D->OnMouseStart(); // Reset mouse tracking for new camera
                return;
            }
        }
        return; // Other keys are not handled by ViewController directly
    }

    ICamera* cam = GetActiveCamera();
    if (!cam) {
        LOG(Warn) << "ViewController: No active camera to process input.";
        return;
    }

    // Dispatch mouse events to active camera
    if (ev.type == InputEventType::MouseButton) {
        const auto& e = std::get<MouseButtonEvent>(ev.data);
        if (e.button == MouseButton::Left) {
            m_LMB = e.pressed;
        }
        if (e.button == MouseButton::Right) {
            m_RMB = e.pressed;
        }
        if (e.pressed) {  // On any button press, signal camera to start mouse tracking
            cam->OnMouseStart();
        }
        return;
    }

    if (ev.type == InputEventType::MouseMove) {
        auto& mm = std::get<MouseMoveEvent>(ev.data);
        if (m_LMB) { // Left Mouse Button for Rotation
            cam->OnMouseRotation(mm.position.x, mm.position.y);
        } else if (m_RMB) { // Right Mouse Button for Panning
            cam->OnMousePan(mm.position.x, mm.position.y);
        }
        return;
    }

    if (ev.type == InputEventType::Scroll) {
        auto& sc = std::get<ScrollEvent>(ev.data);
        cam->OnMouseScroll(sc.offset.y);
        return;
    }
}

/**
 * \brief Updates the active camera's state.
 * \param dt Time delta since the last update.
 */
void ViewController::Update(double dt) {
    if (auto* cam = GetActiveCamera())
        cam->Update(static_cast<float>(dt));
}

}  // namespace of::editor
