#pragma once

#include <glm/vec2.hpp>
#include <nlohmann/json.hpp>
#include <ontoflow/domain/Components.hpp>
#include <ontoflow/domain/Registry.hpp>
#include <optional>
#include <string>
#include <vector>

namespace of::engine {

/**
 * @brief A serialized representation of a single pin within a node.
 *
 * This DTO is detached from any ECS state and can be safely serialized
 * to/from JSON. After loading, it will be mapped to ECS entities.
 */
struct SerializedPin {
    std::string name;
    of::domain::PinType type = of::domain::PinType::ANY;
    of::domain::PinValue value;  ///< Default input value or output value.

    of::domain::EntityID targetNodeId = of::domain::INVALID_ENTITY_ID;
    std::size_t targetPinIndex = 0;
};

/**
 * @brief A complete serialized representation of a node in the graph.
 */
struct SerializedNode {
    of::domain::EntityID id = of::domain::INVALID_ENTITY_ID;  ///< Local stable graph ID.
    std::string definitionId;                                 ///< Matches a NodeDefinition.
    std::string displayName;                                  ///< Custom name component.

    glm::vec2 uiPos{0.0f, 0.0f};  ///< Node position in editor space.

    std::vector<SerializedPin> inputs;
    std::vector<SerializedPin> outputs;
};

/**
 * @brief Root object for a serialized computational graph.
 */
struct GraphDocument {
    std::string name;
    std::vector<SerializedNode> nodes;
};

/**
 * @brief Responsible for capturing a live ECS graph into a serializable format
 *        and reconstructing it later.
 *
 * Separation of concerns:
 *  - Conversion ECS -> DTO (CaptureGraph)
 *  - Conversion DTO -> ECS (ApplyGraph)
 *  - Serialization DTO <-> JSON (Save, Load)
 *
 * The GraphSerializer does *not* own any ECS entities.
 */
class GraphSerializer {
   public:
    explicit GraphSerializer(of::domain::Registry& reg);

    /**
     * @brief Extracts all nodes from the ECS registry and converts
     *        them to an independent DTO (GraphDocument).
     */
    GraphDocument CaptureGraph(const std::string& graphName = "Untitled");

    /**
     * @brief Reconstructs nodes + connections inside the ECS registry
     *        from a serialized document.
     *
     * Existing graph content is NOT removed automatically.
     * Users should clear the registry before loading a new graph.
     */
    void ApplyGraph(const GraphDocument& doc);

    /**
     * @brief Serialize a GraphDocument to JSON.
     */
    static nlohmann::json ToJson(const GraphDocument& doc);

    /**
     * @brief Parse a GraphDocument from JSON.
     */
    static GraphDocument FromJson(const nlohmann::json& j);

    /**
     * @brief Save the graph document to a JSON file.
     */
    bool SaveToFile(const std::string& path, const GraphDocument& doc);

    /**
     * @brief Load and parse a graph document from a JSON file.
     */
    std::optional<GraphDocument> LoadFromFile(const std::string& path);

   private:
    of::domain::Registry& m_Reg;
};

}  // namespace of::engine
