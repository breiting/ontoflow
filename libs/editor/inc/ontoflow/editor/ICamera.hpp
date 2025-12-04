#pragma once
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace of::editor {

/**
 * \brief Interface for a generic camera in the editor.
 *
 * This abstract class defines the common functionality expected from any camera
 * implementation (e.g., 2D orthographic, 3D perspective). It provides methods
 * for retrieving view/projection matrices, handling input events, and updating
 * its state.
 */
class ICamera {
   public:
    /**
     * \brief Virtual destructor to ensure proper cleanup of derived camera classes.
     */
    virtual ~ICamera() = default;

    /**
     * \brief Returns the camera's view matrix.
     * \return A glm::mat4 representing the camera's view transformation.
     */
    virtual glm::mat4 GetViewMatrix() const = 0;

    /**
     * \brief Returns the camera's projection matrix.
     * \return A glm::mat4 representing the camera's projection.
     */
    virtual glm::mat4 GetProjectionMatrix() const = 0;

    /**
     * \brief Returns the camera's view direction vector.
     * \return A normalized glm::vec3 representing the direction the camera is looking.
     */
    virtual glm::vec3 GetViewDirection() const = 0;

    /**
     * \brief Sets the viewport dimensions for the camera.
     * \param w Width of the viewport in pixels.
     * \param h Height of the viewport in pixels.
     */
    virtual void SetViewport(int w, int h) = 0;

    /**
     * \brief Called when a mouse interaction sequence starts (e.g., mouse button pressed).
     * Used to initialize internal state for delta calculations.
     */
    virtual void OnMouseStart() = 0;

    /**
     * \brief Handles mouse rotation input.
     * \param x X-coordinate of the mouse.
     * \param y Y-coordinate of the mouse.
     */
    virtual void OnMouseRotation(double x, double y) = 0;

    /**
     * \brief Handles mouse panning input.
     * \param x X-coordinate of the mouse.
     * \param y Y-coordinate of the mouse.
     */
    virtual void OnMousePan(double x, double y) = 0;

    /**
     * \brief Handles mouse scroll input.
     * \param yoffset Vertical scroll offset.
     */
    virtual void OnMouseScroll(double yoffset) = 0;

    /**
     * \brief Converts screen coordinates to 3D world coordinates.
     * The exact interpretation depends on the camera type (e.g., picking on a plane).
     * \param screenX X-coordinate on screen.
     * \param screenY Y-coordinate on screen.
     * \return A glm::vec3 representing the world coordinates.
     */
    virtual glm::vec3 ScreenToWorld(double screenX, double screenY) const = 0;

    /**
     * \brief Updates the camera's internal state (e.g., damping, interpolation).
     * \param dt Time delta since the last update.
     */
    virtual void Update(float dt) = 0;
};

}  // namespace of::editor
