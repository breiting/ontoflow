#pragma once
#include <memory>
#include <ontoflow/domain/Entity.hpp>
#include <ontoflow/domain/GeometrySystem.hpp>
#include <ontoflow/editor/InputEvent.hpp>
#include <ontoflow/editor/ViewController.hpp>
#include <ontoflow/engine/GraphEvaluator.hpp>
#include <string>

// Forward declaration
namespace of::ui {
class GraphEditorSystem;
}

namespace of::editor {

/**
 * \brief The central editor class managing modes, tools, and input dispatch.
 *
 * The Editor orchestrates user interaction by maintaining a current `EditorMode`
 * and dispatching input events to the active `ITool`. It also manages the
 * `ViewController` for camera control and processes a Vim-like command buffer.
 */
class Editor {
   public:
    /**
     * \brief Constructs an Editor instance.
     */
    explicit Editor(domain::Registry& registry, domain::GeometrySystem& geometrySystem);
    ~Editor();  // Destructor needed for unique_ptr forward declaration

    /**
     * \brief Called once per frame (or simulation step) to update active tools and camera.
     * \param dt Time delta since last frame.
     */
    void Update(double dt);

    /**
     * \brief Initializes the "Pulsating Box" demo graph.
     */
    void InitializeDemoGraph();

    /**
     * \brief Draws the UI panels.
     */
    void DrawUI();

    /**
     * \brief Feeds an input event into the editor for processing.
     * Events are dispatched first to the active tool, then to the camera controller.
     * \param ev The input event.
     */
    void OnInput(const InputEvent& ev);

    // Adapter for ViewController
    /**
     * \brief Sets the 2D camera for the ViewController.
     * \param cam A shared pointer to the 2D camera implementation.
     */
    void SetCamera2D(std::shared_ptr<ICamera> cam);

    /**
     * \brief Sets the 3D camera for the ViewController.
     * \param cam A shared pointer to the 3D camera implementation.
     */
    void SetCamera3D(std::shared_ptr<ICamera> cam);

    /**
     * \brief Sets the viewport size for the ViewController's cameras.
     * \param w Width of the viewport.
     * \param h Height of the viewport.
     */
    void SetViewportSize(int w, int h);

    /**
     * \brief Returns a pointer to the currently active camera.
     * \return A pointer to the ICamera.
     */
    ICamera* GetActiveCamera();

   private:
    domain::Registry& m_Registry;              ///< Reference to the ECS registry.
    domain::GeometrySystem& m_GeometrySystem;  ///< Reference to the geometry system.

    ViewController m_ViewController;  ///< Manages camera views and input dispatch.

    std::string m_CommandBuffer;  ///< Buffer for Vim-like key sequences.

    std::unique_ptr<ui::GraphEditorSystem> m_GraphEditorSystem;  ///< The graph editor system.

    // --- Demo Graph State ---
    std::unique_ptr<of::engine::GraphEvaluator> m_Evaluator;
    domain::EntityID m_WidthNodeID{domain::INVALID_ENTITY_ID};
    domain::EntityID m_BoxNodeID{domain::INVALID_ENTITY_ID};
    bool m_NeedsEvaluation{false};

    void SyncMeshes();

    /**
     * \brief Handles keyboard events, including global shortcuts and command buffer input.
     * \param key The KeyEvent to process.
     */
    void HandleKey(const KeyEvent& key);

    /**
     * \brief Processes the current content of the command buffer.
     * Triggers mode switches based on recognized commands.
     */
    void ProcessCommandBuffer();
};

}  // namespace of::editor
