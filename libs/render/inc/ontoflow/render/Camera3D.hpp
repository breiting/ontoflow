#pragma once
#include <glm/glm.hpp>
#include <ontoflow/editor/ICamera.hpp>

namespace of::render {

/**
 * \brief Implements a 3D orbital camera for scene navigation.
 *
 * This camera orbits around a target point, allowing rotation (pitch, yaw),
 * panning, and zooming. It supports smooth movement with damping.
 */
class Camera3D : public editor::ICamera {
   public:
    /**
     * \brief Constructs a 3D camera.
     * \param radius Initial distance from the target.
     * \param pitch Initial pitch angle in degrees.
     * \param yaw Initial yaw angle in degrees.
     */
    Camera3D(float radius = 15.0f, float pitch = 30.0f, float yaw = -75.0f);

    /**
     * \brief Called when a mouse interaction starts.
     * Resets internal mouse tracking for delta calculation.
     */
    void OnMouseStart() override;

    /**
     * \brief Handles mouse rotation input.
     * Adjusts the camera's pitch and yaw based on mouse movement.
     * \param xpos Current X-coordinate of the mouse.
     * \param ypos Current Y-coordinate of the mouse.
     */
    void OnMouseRotation(double xpos, double ypos) override;

    /**
     * \brief Handles mouse panning input.
     * Moves the camera's target point based on mouse movement.
     * \param xpos Current X-coordinate of the mouse.
     * \param ypos Current Y-coordinate of the mouse.
     */
    void OnMousePan(double xpos, double ypos) override;

    /**
     * \brief Handles mouse scroll input for zooming.
     * Adjusts the camera's distance (radius) from the target.
     * \param yoffset The scroll offset (usually vertical).
     */
    void OnMouseScroll(double yoffset) override;

    /**
     * \brief Updates the camera's internal state, applying damping to rotation.
     * \param deltaTime Time delta since last frame.
     */
    void Update(float deltaTime) override;

    /**
     * \brief Returns the view matrix for the 3D camera.
     * \return A glm::mat4 representing the camera's view transformation.
     */
    glm::mat4 GetViewMatrix() const override;

    /**
     * \brief Returns the projection matrix for the 3D camera.
     * \return A glm::mat4 representing the perspective projection.
     */
    glm::mat4 GetProjectionMatrix() const override;

    /**
     * \brief Returns the view direction of the camera.
     * \return A glm::vec3 representing the normalized vector from position to target.
     */
    glm::vec3 GetViewDirection() const override;

    /**
     * \brief Sets the absolute position of the camera.
     * \param position The new position vector.
     */
    void SetPosition(const glm::vec3& position);

    /**
     * \brief Sets the target point the camera is looking at.
     * \param target The new target point vector.
     */
    void SetTarget(const glm::vec3& target);

    /**
     * \brief Returns the current position of the camera.
     * \return A const reference to the camera's position.
     */
    const glm::vec3& GetPosition() const;

    /**
     * \brief Returns the current target point of the camera.
     * \return A const reference to the camera's target.
     */
    const glm::vec3& GetTarget() const;

    /**
     * \brief Converts screen coordinates to 3D world coordinates using ray picking.
     * This method casts a ray from the camera through the screen point and calculates
     * its intersection with the XY-plane (Z=0).
     * \param screenX X-coordinate on screen.
     * \param screenY Y-coordinate on screen.
     * \return A glm::vec3 representing the world coordinates on the XY-plane.
     */
    glm::vec3 ScreenToWorld(double screenX, double screenY) const override;

    /**
     * \brief Sets the viewport dimensions.
     * \param w Width of the viewport in pixels.
     * \param h Height of the viewport in pixels.
     */
    void SetViewport(int w, int h) override {
        m_VP = {float(w), float(h)};
        m_AspectRatio = float(w) / float(h);
    }

   private:
    /**
     * \brief Calculates mouse movement delta.
     * \param xpos Current X-coordinate of the mouse.
     * \param ypos Current Y-coordinate of the mouse.
     * \return A glm::vec2 representing the mouse delta.
     */
    glm::vec2 OnMouseMove(double xpos, double ypos);

    /**
     * \brief Updates the camera's position based on its target, radius, and rotation.
     */
    void UpdatePosition();

    glm::vec2 m_Rotation; ///< Current rotation angles (yaw, pitch) in degrees.
    glm::vec2 m_RotationVelocity; ///< Current rotation velocity for damping.
    glm::vec2 m_VP; ///< Viewport dimensions (width, height).
    float m_Radius; ///< Distance from the target point.
    float m_DampingFactor; ///< Factor to apply for rotation damping.
    float m_VelocityThreshold; ///< Threshold below which rotation velocity is set to zero.
    glm::vec2 m_LastMouse; ///< Last known mouse position for delta calculation.
    bool m_FirstMouse; ///< Flag to initialize m_LastMouse on first mouse event.

    glm::vec3 m_Position{0.0f, 0.0f, 5.0f}; ///< Current camera position in world space.
    glm::vec3 m_Target{0.0f, 0.0f, 0.0f}; ///< Point the camera is looking at.
    float m_AspectRatio{1.0f}; ///< Aspect ratio of the viewport.
};

}  // namespace of::render
