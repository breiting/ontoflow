## 🛠️ Task: Implement Modular Standard Node Library

**Context:**
We are establishing the standard library of nodes for OntoFlow. Instead of a monolithic file, we need a modular structure separated by domain (Math, Geometry, Logic) with a central registration facade. We should also put every node into one file.

**Goal:**

1.  Create modular headers/sources for node categories.
2.  Implement `StandardLibrary` class for one-line registration.
3.  Update `main.cpp` to use the clean API for the headless project.

### 1\. 📂 File Structure Update

Create the following files:

- `libs/nodes/inc/ontoflow/nodes/InputNodes.hpp`
- `libs/nodes/inc/ontoflow/nodes/MathNodes.hpp`
- `libs/nodes/inc/ontoflow/nodes/GeometryNodes.hpp`
- `libs/nodes/inc/ontoflow/nodes/StandardLibrary.hpp`
- `libs/nodes/src/nodes/MathNodes.cpp`
- `libs/nodes/src/nodes/GeometryNodes.cpp`
- `libs/nodes/src/nodes/StandardLibrary.cpp`

### 🧮 Implementation: Input Nodes

```cpp
#pragma once

namespace of::nodes {

/**
 * \brief Registers all input nodes (Float, Int, Vector, etc.)
 */
void RegisterInputNodes();

}
```

**`src/nodes/InputNodes.cpp`**

```cpp
#include <ontoflow/engine/NodeRegistry.hpp>
#include <ontoflow/domain/Components.hpp>
#include <variant>

namespace of::nodes {

using namespace of::domain;
using namespace of::engine;

void RegisterInputNodes() {
    auto& reg = NodeRegistry::Instance();

    // --- Float Value Node ---
    NodeDefinition valDef;
    valDef.name = "Value (Float)";
    valDef.category = "Input";
    valDef.outputs.push_back(Pin{"Out", PinType::FLOAT});
    valDef.compute = [](NodeComponent&, Registry&) { /* Static value passed through */ };
    reg.RegisterNode("FLOAT_VALUE", valDef);

    // --- Int Value Node ---
    NodeDefinition valDef;
    valDef.name = "Value (Int)";
    valDef.category = "Input";
    valDef.outputs.push_back(Pin{"Out", PinType::INT});
    valDef.compute = [](NodeComponent&, Registry&) { /* Static value passed through */ };
    reg.RegisterNode("INT_VALUE", valDef);
}

}
```

### 2\. 🧮 Implementation: Math Nodes

**`inc/ontoflow/nodes/MathNodes.hpp`**

```cpp
#pragma once

namespace of::nodes {

/**
 * \brief Registers all mathematical nodes (Value, Add, Sin, etc.)
 */
void RegisterMathNodes();

}
```

**`src/nodes/MathNodes.cpp`**

```cpp
#include <ontoflow/nodes/MathNodes.hpp>
#include <ontoflow/engine/NodeRegistry.hpp>
#include <ontoflow/domain/Components.hpp>
#include <variant>

namespace of::nodes {

using namespace of::domain;
using namespace of::engine;

void RegisterMathNodes() {
    auto& reg = NodeRegistry::Instance();

    // --- Add Node ---
    NodeDefinition addDef;
    addDef.name = "Add";
    addDef.category = "Math";
    addDef.inputs.push_back(Pin{"A", PinType::FLOAT});
    addDef.inputs.push_back(Pin{"B", PinType::FLOAT});
    addDef.outputs.push_back(Pin{"Result", PinType::FLOAT});

    addDef.compute = [](NodeComponent& node, Registry&) {
        double a = 0.0;
        double b = 0.0;

        if (auto* v = std::get_if<double>(&node.inputs[0].value)) a = *v;
        if (auto* v = std::get_if<double>(&node.inputs[1].value)) b = *v;

        node.outputs[0].value = a + b;
    };
    reg.RegisterNode("MATH_ADD", addDef);
}

}
```

### 3\. 📐 Implementation: Geometry Nodes

**`inc/ontoflow/nodes/GeometryNodes.hpp`**

```cpp
#pragma once
#include <ontoflow/geometry/GeometryBackend.hpp>

namespace of::nodes {

/**
 * \brief Registers geometry generation nodes. Needs backend access.
 */
void RegisterGeometryNodes(of::geometry::GeometryBackend& backend);

}
```

**`src/nodes/GeometryNodes.cpp`**

```cpp
#include <ontoflow/nodes/GeometryNodes.hpp>
#include <ontoflow/engine/NodeRegistry.hpp>
#include <ontoflow/domain/Components.hpp>

namespace of::nodes {

using namespace of::domain;
using namespace of::engine;

void RegisterGeometryNodes(of::geometry::GeometryBackend& backend) {
    auto& reg = NodeRegistry::Instance();

    // --- Box Node ---
    NodeDefinition boxDef;
    boxDef.name = "Box Generator";
    boxDef.category = "Geometry";
    boxDef.inputs = {
        {"Width", PinType::FLOAT},
        {"Length", PinType::FLOAT},
        {"Height", PinType::FLOAT}
    };
    boxDef.outputs = { {"Shape", PinType::GEOMETRY} };

    // Capture backend by reference
    boxDef.compute = [&backend](NodeComponent& node, Registry& r) {
        double w=10, l=10, h=10;

        if (auto* v = std::get_if<double>(&node.inputs[0].value)) w = *v;
        if (auto* v = std::get_if<double>(&node.inputs[1].value)) l = *v;
        if (auto* v = std::get_if<double>(&node.inputs[2].value)) h = *v;

        // 1. Get/Create Body Entity (Reuse logic)
        EntityID bodyEnt = INVALID_ENTITY_ID;

        if (auto* handle = std::get_if<GeometryHandle>(&node.outputs[0].value)) {
            if (handle->IsValid() && r.HasComponent<BodyComponent>(handle->id)) {
                bodyEnt = handle->id;
            }
        }

        if (bodyEnt == INVALID_ENTITY_ID) {
            bodyEnt = r.CreateEntity();
            r.AddComponent(bodyEnt, BodyComponent{});
            r.AddComponent(bodyEnt, NameComponent{"Box_Body"});
            node.outputs[0].value = GeometryHandle{bodyEnt};
        }

        // 2. OCCT Calculation
        auto shapeHandle = backend.MakeBox(w, l, h);

        // 3. Update Body
        r.GetComponent<BodyComponent>(bodyEnt)->handle = shapeHandle;
    };
    reg.RegisterNode("GEO_BOX", boxDef);
}

}
```

### 4\. 📚 Implementation: Standard Library Facade

**`inc/ontoflow/nodes/StandardLibrary.hpp`**

```cpp
#pragma once
#include <ontoflow/geometry/GeometryBackend.hpp>

namespace of::nodes {

/**
 * \brief Facade class to register all available node libraries at once.
 */
class StandardLibrary {
public:
    static void RegisterAll(of::geometry::GeometryBackend& backend);
};

}
```

**`src/nodes/StandardLibrary.cpp`**

```cpp
#include <ontoflow/nodes/StandardLibrary.hpp>
#include <ontoflow/nodes/MathNodes.hpp>
#include <ontoflow/nodes/GeometryNodes.hpp>

namespace of::nodes {

void StandardLibrary::RegisterAll(of::geometry::GeometryBackend& backend) {
    RegisterMathNodes();
    RegisterGeometryNodes(backend);
    // Future: RegisterLogicNodes(), RegisterAgentNodes()...
}

}
```

### 5\. 🚀 Update Main (The Clean Result)

Update `apps/headless/main.cpp` to use the new facade.

```cpp
// ... includes ...
#include <ontoflow/nodes/StandardLibrary.hpp>

int main() {
    // ... Init Registry & Backend ...

    // 2. Load Knowledge (One-Liner!)
    of::nodes::StandardLibrary::RegisterAll(backend);

    // ... Graph Construction ...
}
```

**Deliverables:**
Implement the file structure and code as defined above. Ensure compilation succeeds.
