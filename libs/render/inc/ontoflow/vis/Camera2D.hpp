#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <ontoflow/editor/ICamera.hpp>

namespace of::vis {

/**
 * \brief Implements a 2D orthographic camera for editor views.
 *
 * This camera provides panning and zooming functionality, and translates
 * screen coordinates to world coordinates suitable for 2D sketching.
 */
class Camera2D : public editor::ICamera {
   public:
    /**
     * \brief Constructs a default 2D camera.
     * Initializes position to origin, zoom to 5.0, and aspect ratio to 1.0.
     */
    Camera2D();

    // --- ICamera Interface ---
    /**
     * \brief Called when a mouse interaction starts.
     * Resets internal mouse tracking for delta calculation.
     */
    void OnMouseStart() override;

    /**
     * \brief Handles mouse rotation input (ignored in 2D camera).
     * \param xpos Current X-coordinate of the mouse.
     * \param ypos Current Y-coordinate of the mouse.
     */
    void OnMouseRotation(double /*xpos*/, double /*ypos*/) override {
    }  // No rotation in 2D

    /**
     * \brief Handles mouse panning input.
     * Adjusts the camera's position based on mouse movement.
     * \param xpos Current X-coordinate of the mouse.
     * \param ypos Current Y-coordinate of the mouse.
     */
    void OnMousePan(double xpos, double ypos) override;

    /**
     * \brief Handles mouse scroll input for zooming.
     * Adjusts the camera's zoom level.
     * \param yoffset The scroll offset (usually vertical).
     */
    void OnMouseScroll(double yoffset) override;

    /**
     * \brief Updates the camera's internal state (ignored for 2D camera as it's purely event-driven).
     * \param dt Time delta since last frame.
     */
    void Update(float /*dt*/) override {
    }

    /**
     * \brief Sets the viewport dimensions.
     * \param w Width of the viewport in pixels.
     * \param h Height of the viewport in pixels.
     */
    void SetViewport(int w, int h) override {
        m_VP = {float(w), float(h)};
        m_AspectRatio = float(w) / float(h);
    }

    /**
     * \brief Returns the view matrix for the 2D camera.
     * \return A glm::mat4 representing the camera's view transformation.
     */
    glm::mat4 GetViewMatrix() const override;

    /**
     * \brief Returns the projection matrix for the 2D camera.
     * \return A glm::mat4 representing the orthographic projection.
     */
    glm::mat4 GetProjectionMatrix() const override;

    /**
     * \brief Returns the view direction of the camera (always looking down -Z in 2D).
     * \return A glm::vec3 representing the view direction.
     */
    glm::vec3 GetViewDirection() const override {
        return {0, 0, -1};
    }

    // Helpers
    /**
     * \brief Sets the 2D position of the camera.
     * \param pos The new 2D position.
     */
    void SetPosition(const glm::vec2& pos);

    /**
     * \brief Returns the current 2D position of the camera.
     * \return A const reference to the glm::vec2 position.
     */
    const glm::vec2& GetPosition() const {
        return m_Position;
    }

    /**
     * \brief Returns the current zoom level of the camera.
     * \return The zoom factor.
     */
    float GetZoom() const {
        return m_Zoom;
    }

    /**
     * \brief Converts screen coordinates to 3D world coordinates (on the XY-plane).
     * \param screenX X-coordinate on screen.
     * \param screenY Y-coordinate on screen.
     * \return A glm::vec3 representing the world coordinates.
     */
    glm::vec3 ScreenToWorld(double screenX, double screenY) const override;

   private:
    glm::vec2 m_Position; ///< Current 2D position of the camera target.
    glm::vec2 m_LastMouse; ///< Last known mouse position for pan delta calculation.
    glm::vec2 m_VP; ///< Viewport dimensions (width, height).
    bool m_FirstMouse; ///< Flag to initialize m_LastMouse on first mouse event.
    float m_Zoom; ///< Current zoom level.
    float m_AspectRatio; ///< Aspect ratio of the viewport.
};

}  // namespace of::vis
