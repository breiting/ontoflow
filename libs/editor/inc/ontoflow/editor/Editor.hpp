#pragma once
/**
 * @file Editor.hpp
 * @brief Main UI/editor controller for OntoFlow. Integrates viewport, node editor,
 *        cameras, command stack, and geometry updates.
 */

#include <memory>
#include <ontoflow/domain/GeometrySystem.hpp>
#include <ontoflow/domain/Registry.hpp>
#include <ontoflow/editor/IViewportRenderer.hpp>
#include <ontoflow/editor/ViewController.hpp>
#include <ontoflow/engine/GraphEvaluator.hpp>
#include <ontoflow/ui/GraphEditorSystem.hpp>
#include <ontoflow/ui/NodeEditorRegistry.hpp>
#include <ontoflow/ui/UiIdAllocator.hpp>
#include <string>

namespace of::editor {

/**
 * @class Editor
 * @brief Top–level orchestrator for UI and graph evaluation.
 *
 * This class owns:
 *  - GraphEditorSystem (node editor UI)
 *  - GraphEvaluator (dataflow runtime)
 *  - ViewController (camera & tools)
 *  - GeometrySystem (backend geometry)
 */
class Editor {
   public:
    Editor(domain::Registry& registry, domain::GeometrySystem& geometrySystem, IViewportRenderer* renderer);
    ~Editor();

    void DrawUI();
    void Update(double dt);

    void Initialize();
    void ExecuteCommand(const std::string& cmd);

    void SetCamera2D(std::shared_ptr<ICamera> cam);
    void SetCamera3D(std::shared_ptr<ICamera> cam);
    void SetViewportSize(int w, int h);
    ICamera* GetActiveCamera();

    void OnInput(const InputEvent& ev);

    // void InitializeDemoGraph();

   private:
    void HandleKey(const KeyEvent& key);
    void SyncMeshes();

   private:
    domain::Registry& m_Registry;
    domain::GeometrySystem& m_GeometrySystem;
    IViewportRenderer* m_Renderer = nullptr;

    // --- Core systems ---
    std::unique_ptr<engine::GraphEvaluator> m_Evaluator;

    // UI Editors
    ui::UiIdAllocator m_UiAllocator;
    ui::NodeEditorRegistry m_NodeEditorRegistry;
    std::unique_ptr<ui::GraphEditorSystem> m_GraphEditorSystem;

    // Cameras & Tools
    ViewController m_ViewController;

    // Graph demo nodes
    domain::Entity m_WidthNodeID = domain::INVALID_ENTITY;
    domain::Entity m_SinkNodeID = domain::INVALID_ENTITY;

    bool m_NeedsEvaluation = true;

    // Minimal shortcut system (kept as-is)
    std::string m_CommandBuffer;

    bool m_ShowRenderWindow{false};
};

}  // namespace of::editor
