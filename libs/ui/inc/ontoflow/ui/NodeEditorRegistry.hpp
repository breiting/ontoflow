#pragma once
/**
 * @file NodeEditorRegistry.hpp
 * @brief Maintains UI IDs and mappings between UI IDs and ECS nodes/pins.
 */

#include <ontoflow/domain/Entity.hpp>
#include <ontoflow/ui/UiIdAllocator.hpp>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace of::ui {

/**
 * @class NodeEditorRegistry
 * @brief Central mapping between ECS entities (nodes), pins, and UI IDs.
 *
 * This registry provides:
 *  - unique node IDs for ImNodes
 *  - unique pin IDs for each (node, pinIndex, direction)
 *  - unique link IDs based on the input pin
 */
class NodeEditorRegistry {
   public:
    explicit NodeEditorRegistry(UiIdAllocator& alloc);

    // ------------------------------------------------------------------------
    // Node ID handling
    // ------------------------------------------------------------------------

    /**
     * @brief Get or allocate a UI node ID for an ECS entity.
     */
    int GetNodeId(of::domain::Entity entity);

    /**
     * @brief Given a UI node ID, return associated ECS entity.
     */
    of::domain::Entity GetNodeEntity(int uiNodeId) const;

    // ------------------------------------------------------------------------
    // Pin ID handling
    // ------------------------------------------------------------------------

    /**
     * @struct PinKey
     * @brief Unique identifier for an ECS pin location.
     */
    struct PinKey {
        of::domain::Entity node;
        std::size_t pinIndex;
        bool isOutput;

        bool operator==(const PinKey& o) const {
            return node == o.node && pinIndex == o.pinIndex && isOutput == o.isOutput;
        }
    };

    struct PinKeyHash {
        std::size_t operator()(const PinKey& k) const;
    };

    /**
     * @brief Get or allocate a UI pin ID for a specific ECS pin.
     */
    int GetPinId(of::domain::Entity node, std::size_t pinIndex, bool isOutput);

    /**
     * @brief Given a UI pin ID, return its associated PinKey.
     */
    PinKey DecodePin(int uiPinId) const;

    // ------------------------------------------------------------------------
    // Link ID handling
    // ------------------------------------------------------------------------

    /**
     * @struct LinkKey
     * @brief Unique identifier for an ECS input pin used for a link.
     *
     * One input pin can have at most one link visualized in the editor.
     */
    struct LinkKey {
        of::domain::Entity node;
        std::size_t pinIndex;

        bool operator==(const LinkKey& o) const {
            return node == o.node && pinIndex == o.pinIndex;
        }
    };

    struct LinkKeyHash {
        std::size_t operator()(const LinkKey& k) const;
    };

    /**
     * @brief Get or allocate a UI link ID for a specific ECS input pin.
     */
    int GetLinkId(of::domain::Entity inputNode, std::size_t inputPin);

    /**
     * @brief Given a UI link ID, decode the associated (entity, pinIndex).
     */
    std::pair<of::domain::Entity, std::size_t> DecodeLink(int uiLinkId) const;

    bool HasSeenNode(int nodeId) const;
    void MarkNodeSeen(int nodeId);

   private:
    UiIdAllocator& m_alloc;

    // Nodes
    std::unordered_map<of::domain::Entity, int> m_nodeToUi;
    std::unordered_map<int, of::domain::Entity> m_uiToNode;

    // Pins
    std::unordered_map<PinKey, int, PinKeyHash> m_pinToUi;
    std::unordered_map<int, PinKey> m_uiToPin;

    // Links
    std::unordered_map<LinkKey, int, LinkKeyHash> m_linkToUi;
    std::unordered_map<int, LinkKey> m_uiToLink;

    std::unordered_set<int> m_seenNodes;
};

}  // namespace of::ui
