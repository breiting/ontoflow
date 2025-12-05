#pragma once

#include <ontoflow/domain/Components.hpp>
#include <ontoflow/domain/Registry.hpp>
#include <string>

namespace of::ui {

enum class InspectorAction {
    None,
    Save,
    Cancel,
    Delete
};

class NodeInspector {
   public:
    void Open(domain::Entity e, domain::Registry& reg);
    InspectorAction Draw(domain::Registry& reg);
    
    domain::Entity GetTarget() const { return m_Target; }

   private:
    void DrawHeader();
    void DrawGeneralSection();
    void DrawPinsSection();
    void DrawActionsFooter();

    // Helper for rendering connection status
    void DrawConnectionStatus(bool connected);

    // Helper for rendering specific pin editors
    void DrawPinEditor(domain::Pin& pin);

   private:
    bool m_IsOpen = false;
    domain::Entity m_Target = domain::INVALID_ENTITY;

    // Temporary Transaction State
    domain::NodeComponent m_TempNode;
    std::string m_TempName;
};

}  // namespace of::ui
