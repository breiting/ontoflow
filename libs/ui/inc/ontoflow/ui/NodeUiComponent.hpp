#pragma once
/**
 * @file NodeUiComponent.hpp
 * @brief Stores UI metadata for rendering a node inside ImNodes.
 */

#include <glm/vec2.hpp>
#include <vector>

namespace of::ui {

/**
 * @struct NodeUiComponent
 * @brief Holds UI state used by the node editor (position, UI IDs).
 *
 * This component is editor-only and does not influence dataflow logic.
 */
struct NodeUiComponent {
    int uiNodeId = -1;              ///< ImNodes unique node ID
    glm::vec2 pos{0.0f, 0.0f};      ///< Node position in ImNodes grid space
    std::vector<int> inputPinIds;   ///< UI pin IDs for all input pins
    std::vector<int> outputPinIds;  ///< UI pin IDs for all output pins
};

}  // namespace of::ui
