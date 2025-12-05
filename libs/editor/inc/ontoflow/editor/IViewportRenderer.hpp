#pragma once
#include <ontoflow/editor/ICamera.hpp>

namespace of::editor {

struct Viewport {
    int x;
    int y;
    int width;
    int height;
};

/**
 * \brief Generic callback interface for rendering into an ImGui viewport region.
 *
 * Editors are UI-only and should not depend on specific rendering backends
 * (OpenGL, Vulkan, Metal, etc.). The concrete implementation lives in
 * ontoflow::render or any other rendering module.
 */
class IViewportRenderer {
   public:
    virtual ~IViewportRenderer() = default;

    /**
     * \brief Render the current scene into the given viewport region.
     * \param cam   The active camera.
     * \param vp    The screen-space viewport region.
     */
    virtual void Render(ICamera* cam, const Viewport& vp) = 0;
};

}  // namespace of::editor
