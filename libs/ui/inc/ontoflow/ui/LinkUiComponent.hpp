#pragma once
/**
 * @file LinkUiComponent.hpp
 * @brief Stores UI metadata for rendering links between pins.
 */

#include <cstddef>
#include <ontoflow/domain/Types.hpp>

namespace of::ui {

/**
 * @struct LinkUiComponent
 * @brief UI information for a link connecting two pins.
 *
 * The actual dataflow connection lives inside NodeComponent::inputs[i].connection.
 * This component is purely for UI representation.
 */
struct LinkUiComponent {
    int uiLinkId = -1;

    of::domain::EntityID outputNode;
    size_t outputPin;

    of::domain::EntityID inputNode;
    size_t inputPin;
};

}  // namespace of::ui
