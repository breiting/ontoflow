#include <ontoflow/ui/NodeEditorRegistry.hpp>

namespace of::ui {

NodeEditorRegistry::NodeEditorRegistry(UiIdAllocator& alloc) : m_alloc(alloc) {
}

// ---------------------- PinKeyHash ----------------------

std::size_t NodeEditorRegistry::PinKeyHash::operator()(const PinKey& k) const {
    // Simple combination hash; good enough for editor usage.
    std::size_t h1 = std::hash<of::domain::Entity>()(k.node);
    std::size_t h2 = std::hash<std::size_t>()(k.pinIndex);
    std::size_t h3 = std::hash<bool>()(k.isOutput);
    return h1 ^ (h2 << 1) ^ (h3 << 2);
}

// ---------------------- LinkKeyHash ----------------------

std::size_t NodeEditorRegistry::LinkKeyHash::operator()(const LinkKey& k) const {
    std::size_t h1 = std::hash<of::domain::Entity>()(k.node);
    std::size_t h2 = std::hash<std::size_t>()(k.pinIndex);
    return h1 ^ (h2 << 1);
}

// ---------------------- Node IDs ----------------------

int NodeEditorRegistry::GetNodeId(of::domain::Entity entity) {
    auto it = m_nodeToUi.find(entity);
    if (it != m_nodeToUi.end())
        return it->second;

    int id = m_alloc.Allocate();
    m_nodeToUi[entity] = id;
    m_uiToNode[id] = entity;
    return id;
}

of::domain::Entity NodeEditorRegistry::GetNodeEntity(int uiNodeId) const {
    auto it = m_uiToNode.find(uiNodeId);
    if (it != m_uiToNode.end())
        return it->second;

    return of::domain::INVALID_ENTITY;
}

// ---------------------- Pin IDs ----------------------

int NodeEditorRegistry::GetPinId(of::domain::Entity node, std::size_t pin, bool isOutput) {
    PinKey key{node, pin, isOutput};
    auto it = m_pinToUi.find(key);
    if (it != m_pinToUi.end())
        return it->second;

    int id = m_alloc.Allocate();
    m_pinToUi[key] = id;
    m_uiToPin[id] = key;
    return id;
}

NodeEditorRegistry::PinKey NodeEditorRegistry::DecodePin(int uiPinId) const {
    auto it = m_uiToPin.find(uiPinId);
    if (it == m_uiToPin.end()) {
        // In a debug build you might assert here.
        return PinKey{of::domain::INVALID_ENTITY, 0u, false};
    }
    return it->second;
}

// ---------------------- Link IDs ----------------------

int NodeEditorRegistry::GetLinkId(of::domain::Entity inputNode, std::size_t inputPin) {
    LinkKey key{inputNode, inputPin};
    auto it = m_linkToUi.find(key);
    if (it != m_linkToUi.end())
        return it->second;

    int id = m_alloc.Allocate();
    m_linkToUi[key] = id;
    m_uiToLink[id] = key;
    return id;
}

std::pair<of::domain::Entity, std::size_t> NodeEditorRegistry::DecodeLink(int uiLinkId) const {
    auto it = m_uiToLink.find(uiLinkId);
    if (it == m_uiToLink.end()) {
        // Again, for safety you could assert in debug.
        return {of::domain::INVALID_ENTITY, 0u};
    }
    const LinkKey& k = it->second;
    return {k.node, k.pinIndex};
}

}  // namespace of::ui
