#include <fstream>
#include <ontoflow/core/Logger.hpp>
#include <ontoflow/engine/GraphSerializer.hpp>
#include <ontoflow/engine/NodeRegistry.hpp>
#include <glm/vec3.hpp>

// Define JSON serialization for custom types in the global namespace or nlohmann::adl_serializer
namespace glm {
    void to_json(nlohmann::json& j, const glm::vec3& v) {
        j = nlohmann::json::array({v.x, v.y, v.z});
    }
    void from_json(const nlohmann::json& j, glm::vec3& v) {
        v.x = j.at(0).get<float>();
        v.y = j.at(1).get<float>();
        v.z = j.at(2).get<float>();
    }
}

namespace of::domain {
    void to_json(nlohmann::json& j, const GeometryHandle& h) {
        j = {{"id", h.id}};
    }
    void from_json(const nlohmann::json& j, GeometryHandle& h) {
        h.id = j.at("id").get<EntityID>();
    }

    void to_json(nlohmann::json& j, const std::monostate&) {
        j = nullptr;
    }
}

using namespace of::domain;

namespace of::engine {

// Helper to serialize the variant
struct PinValueSerializer {
    nlohmann::json& j;
    void operator()(double v) { j = v; }
    void operator()(int v) { j = v; }
    void operator()(bool v) { j = v; }
    void operator()(const glm::vec3& v) { j = v; } // uses glm::to_json
    void operator()(const std::string& v) { j = v; }
    void operator()(const GeometryHandle& v) { j = v; } // uses GeometryHandle::to_json
    void operator()(std::monostate) { j = nullptr; }
};

// ------------------------------------------------------------
// Constructor
// ------------------------------------------------------------
GraphSerializer::GraphSerializer(Registry& reg) : m_Reg(reg) {
}

// ------------------------------------------------------------
// Capture ECS -> DTO
// ------------------------------------------------------------
GraphDocument GraphSerializer::CaptureGraph(const std::string& graphName) {
    GraphDocument doc;
    doc.name = graphName;

    auto entities = m_Reg.GetEntitiesWith<NodeComponent>();

    for (auto e : entities) {
        const NodeComponent* node = m_Reg.GetComponent<NodeComponent>(e);
        const NameComponent* name = m_Reg.GetComponent<NameComponent>(e);

        if (!node)
            continue;

        SerializedNode sn;
        sn.id = e;
        sn.definitionId = node->definitionID;
        sn.displayName = (name ? name->name : "");
        sn.uiPos = node->ui;

        // Inputs
        for (const auto& pin : node->inputs) {
            SerializedPin sp;
            sp.name = pin.name;
            sp.type = pin.type;
            sp.value = pin.value;

            sp.targetNodeId = pin.connection.targetNodeID;
            sp.targetPinIndex = pin.connection.targetPinIdx;

            sn.inputs.push_back(sp);
        }

        // Outputs
        for (const auto& pin : node->outputs) {
            SerializedPin sp;
            sp.name = pin.name;
            sp.type = pin.type;
            sp.value = pin.value;
            sn.outputs.push_back(sp);
        }

        doc.nodes.push_back(std::move(sn));
    }

    return doc;
}

// ------------------------------------------------------------
// Apply DTO -> ECS (Two-pass reconstruction)
// ------------------------------------------------------------
void GraphSerializer::ApplyGraph(const GraphDocument& doc) {
    auto& nodeReg = NodeRegistry::Instance();

    // Mapping: serialized ID -> new ECS ID
    std::unordered_map<EntityID, EntityID> idMap;

    // ---------------- Pass 1: Create nodes ----------------
    for (const auto& sn : doc.nodes) {
        Entity newE = nodeReg.SpawnNode(m_Reg, sn.definitionId, sn.displayName, sn.uiPos);
        idMap[sn.id] = newE;

        // Overwrite default values immediately
        NodeComponent* nc = m_Reg.GetComponent<NodeComponent>(newE);
        if (!nc)
            continue;

        // Inputs
        for (size_t i = 0; i < sn.inputs.size() && i < nc->inputs.size(); ++i) {
            nc->inputs[i].value = sn.inputs[i].value;
        }

        // Outputs
        for (size_t i = 0; i < sn.outputs.size() && i < nc->outputs.size(); ++i) {
            nc->outputs[i].value = sn.outputs[i].value;
        }
    }

    // ---------------- Pass 2: Reconnect ----------------
    for (const auto& sn : doc.nodes) {
        Entity newE = idMap.at(sn.id);
        NodeComponent* nc = m_Reg.GetComponent<NodeComponent>(newE);
        if (!nc)
            continue;

        for (size_t i = 0; i < sn.inputs.size() && i < nc->inputs.size(); ++i) {
            const auto& spi = sn.inputs[i];

            if (spi.targetNodeId == INVALID_ENTITY_ID)
                continue;

            Entity target = idMap.at(spi.targetNodeId);

            nc->inputs[i].connection.targetNodeID = target;
            nc->inputs[i].connection.targetPinIdx = spi.targetPinIndex;
        }

        nc->isDirty = true;
    }

    LOG(Info) << "GraphSerializer: Graph applied successfully (" << doc.nodes.size() << " nodes).";
}

// ------------------------------------------------------------
// JSON Serialization
// ------------------------------------------------------------
nlohmann::json GraphSerializer::ToJson(const GraphDocument& doc) {
    using json = nlohmann::json;

    json jnodes = json::array();

    for (const auto& sn : doc.nodes) {
        json jnode;

        jnode["id"] = sn.id;
        jnode["definitionId"] = sn.definitionId;
        jnode["displayName"] = sn.displayName;
        jnode["uiPos"] = {sn.uiPos.x, sn.uiPos.y};

        json jins = json::array();
        for (const auto& pin : sn.inputs) {
            json jp;
            jp["name"] = pin.name;
            jp["type"] = (int)pin.type;
            
            std::visit(PinValueSerializer{jp["value"]}, pin.value);

            jp["targetNodeId"] = pin.targetNodeId;
            jp["targetPinIndex"] = pin.targetPinIndex;
            jins.push_back(jp);
        }

        json jouts = json::array();
        for (const auto& pin : sn.outputs) {
            json jp;
            jp["name"] = pin.name;
            jp["type"] = (int)pin.type;
            std::visit(PinValueSerializer{jp["value"]}, pin.value);
            jouts.push_back(jp);
        }

        jnode["inputs"] = jins;
        jnode["outputs"] = jouts;

        jnodes.push_back(jnode);
    }

    json root;
    root["name"] = doc.name;
    root["nodes"] = jnodes;
    return root;
}

// ------------------------------------------------------------
// JSON Parsing
// ------------------------------------------------------------
GraphDocument GraphSerializer::FromJson(const nlohmann::json& j) {
    GraphDocument doc;
    doc.name = j.value("name", "Untitled");

    auto parseValue = [](const nlohmann::json& jv, PinType type) -> PinValue {
        if (jv.is_null()) return std::monostate{};
        
        switch (type) {
            case PinType::DOUBLE:   return jv.get<double>();
            case PinType::INT:      return jv.get<int>();
            case PinType::BOOL:     return jv.get<bool>();
            case PinType::STRING:   return jv.get<std::string>();
            case PinType::VEC3:     return jv.get<glm::vec3>();
            case PinType::GEOMETRY: return jv.get<GeometryHandle>();
            case PinType::ANY:      return std::monostate{};
            default:                return std::monostate{};
        }
    };

    for (auto& jnode : j["nodes"]) {
        SerializedNode sn;
        sn.id = jnode["id"];
        sn.definitionId = jnode["definitionId"];
        sn.displayName = jnode.value("displayName", "");

        auto pos = jnode["uiPos"];
        sn.uiPos = {pos[0], pos[1]};

        for (auto& jp : jnode["inputs"]) {
            SerializedPin sp;
            sp.name = jp["name"];
            sp.type = (PinType)jp["type"].get<int>();
            sp.value = parseValue(jp["value"], sp.type);
            sp.targetNodeId = jp["targetNodeId"];
            sp.targetPinIndex = jp["targetPinIndex"];
            sn.inputs.push_back(std::move(sp));
        }

        for (auto& jp : jnode["outputs"]) {
            SerializedPin sp;
            sp.name = jp["name"];
            sp.type = (PinType)jp["type"].get<int>();
            sp.value = parseValue(jp["value"], sp.type);
            sn.outputs.push_back(std::move(sp));
        }

        doc.nodes.push_back(std::move(sn));
    }

    return doc;
}

// ------------------------------------------------------------
// Save/Load File IO
// ------------------------------------------------------------
bool GraphSerializer::SaveToFile(const std::string& path, const GraphDocument& doc) {
    nlohmann::json j = ToJson(doc);
    std::ofstream out(path);
    if (!out) {
        LOG(Error) << "GraphSerializer: Cannot write file " << path;
        return false;
    }
    out << j.dump(4);
    return true;
}

std::optional<GraphDocument> GraphSerializer::LoadFromFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        LOG(Error) << "GraphSerializer: File not found: " << path;
        return std::nullopt;
    }
    nlohmann::json j;
    in >> j;
    return FromJson(j);
}

}  // namespace of::engine
