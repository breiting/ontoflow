#pragma once

#include <glm/vec3.hpp>
#include <ontoflow/domain/Types.hpp>
#include <string>
#include <variant>

namespace of::domain {

enum class PinType {
    DOUBLE,
    INT,
    BOOL,
    VEC3,
    STRING,
    GEOMETRY,
    ANY
};

inline const char* PinToString(PinType t) {
    switch (t) {
        case PinType::DOUBLE:
            return "Double";
        case PinType::INT:
            return "Int";
        case PinType::BOOL:
            return "Bool";
        case PinType::VEC3:
            return "Vec3";
        case PinType::GEOMETRY:
            return "Geometry";
        case PinType::STRING:
            return "String";
        case PinType::ANY:
            return "Any";
        default:
            return "Unknown";
    }
}

// Wrapper to safely store Geometry References
struct GeometryHandle {
    EntityID id{INVALID_ENTITY_ID};

    bool IsValid() const {
        return id != INVALID_ENTITY_ID;
    }

    bool operator==(const GeometryHandle& other) const {
        return id == other.id;
    }

    bool operator!=(const GeometryHandle& other) const {
        return !(*this == other);
    }
};

// The Data flowing through the graph
using PinValue = std::variant<  //
    double,                     ///< DOUBLE
    int,                        ///< INT
    bool,                       ///< BOOL
    glm::vec3,                  ///< VEC3
    std::string,                ///< STRING
    GeometryHandle,             ///< GEOMETRY
    std::monostate              ///< ANY
    >;

// The Edge E = (TargetNode, TargetPin)
struct Connection {
    EntityID targetNodeID = INVALID_ENTITY_ID;  // 0 is invalid
    size_t targetPinIdx = 0;
};

// The Pin (Input or Output slot)
struct Pin {
    std::string name;
    PinType type = PinType::DOUBLE;
    PinValue value;         // Static data or Cache
    Connection connection;  // Topology (Input only)

    /// Default constructor (required for std::vector)
    Pin() = default;

    /// Create a pin without initial value
    Pin(std::string n, PinType t) : name(std::move(n)), type(t), value(std::monostate{}), connection{} {
    }

    /// Create a pin with all fields
    Pin(std::string n, PinType t, PinValue v, Connection c = {})
        : name(std::move(n)), type(t), value(std::move(v)), connection(std::move(c)) {
    }
};

}  // namespace of::domain
