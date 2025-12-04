#pragma once
#include <glm/glm.hpp>
#include <ontoflow/render/Material.hpp>
#include <memory>

namespace of::render {

class BaseGeometry; // Forward declaration
class Light; // Forward declaration

/**
 * \brief Interface for a generic 3D renderer.
 *
 * This abstract class defines the contract for any concrete rendering implementation
 * (e.g., OpenGL, Vulkan, DirectX). It provides methods for setting up the viewport,
 * managing frames, and drawing various types of 3D geometry with associated materials.
 */
class IRenderer {
   public:
    /**
     * \brief Virtual destructor to ensure proper cleanup of derived renderer classes.
     */
    virtual ~IRenderer() = default;

    /**
     * \brief Sets the size of the rendering viewport.
     * \param w Width of the viewport in pixels.
     * \param h Height of the viewport in pixels.
     */
    virtual void SetViewportSize(int w, int h) = 0;

    /**
     * \brief Prepares the renderer for a new frame.
     * Clears buffers and sets up global rendering parameters.
     * \param view The camera's view matrix.
     * \param proj The camera's projection matrix.
     * \param light A shared pointer to the primary light source in the scene.
     */
    virtual void BeginFrame(const glm::mat4& view, const glm::mat4& proj, std::shared_ptr<Light> light) = 0;

    /**
     * \brief Finalizes the current frame.
     * (e.g., swapping buffers, post-processing effects).
     */
    virtual void EndFrame() = 0;

    /**
     * \brief Draws a 3D mesh.
     * \param mesh A shared pointer to the mesh geometry to draw.
     * \param mat A shared pointer to the material to apply.
     * \param model The model matrix for positioning and orienting the mesh.
     */
    virtual void DrawMesh(const std::shared_ptr<BaseGeometry> mesh, std::shared_ptr<Material> mat,
                          const glm::mat4& model) = 0;

    /**
     * \brief Draws a set of lines.
     * \param lines A shared pointer to the line set geometry to draw.
     * \param mat A shared pointer to the material to apply.
     * \param model The model matrix for positioning and orienting the line set.
     */
    virtual void DrawLineSet(const std::shared_ptr<BaseGeometry> lines, std::shared_ptr<Material> mat,
                             const glm::mat4& model) = 0;

    /**
     * \brief Draws a set of points.
     * \param points A shared pointer to the point set geometry to draw.
     * \param mat A shared pointer to the material to apply.
     * \param model The model matrix for positioning and orienting the point set.
     */
    virtual void DrawPoints(const std::shared_ptr<BaseGeometry> points, std::shared_ptr<Material> mat,
                            const glm::mat4& model) = 0;

    /**
     * \brief Toggles wireframe rendering mode.
     */
    virtual void ToggleWireframe() = 0;
};

}  // namespace of::render
