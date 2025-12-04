# 🌌 OntoFlow Core Architecture: The Dataflow Engine

**Version:** 0.1 (MVP)
**Status:** Active
**Context:** System Core / Engine

## Abstract & Philosophy

OntoFlow is not a traditional state-based application but a **Flow-Based Programming (FBP)** environment implemented on top of an **Entity Component System (ECS)**.

The core philosophy is **Functionalism**: The state of the world is not static; it is the transient result of a directed graph evaluation. We model the world as a graph $G = (V, E)$, where:

- $V$ (Vertices) represent **Nodes** (Functional Units).
- $E$ (Edges) represent **Data Connections**.

Unlike monolithic CAD objects, an entity in OntoFlow is merely a container for a `NodeComponent`, which defines its role in the processing network.

## The Data Model (The Anatomy)

The architecture distinguishes strictly between **Topology** (Connections), **Data** (Values), and **Logic** (Execution).

### The Generic Node (`NodeComponent`)

Instead of specific classes (`BoxComponent`, `SinusComponent`), we use a single generic component that describes the processor.

```cpp
struct NodeComponent {
    std::string definitionID;   // Key to the NodeRegistry (e.g., "GEOM_BOX", "MATH_ADD")
    std::vector<Pin> inputs;    // Input slots (Data receivers)
    std::vector<Pin> outputs;   // Output slots (Data providers)
    bool isDirty;               // Re-evaluation flag

    // UI Metadata (Position in Graph Editor)
    float uiX, uiY;
};
```

### The Interface (`Pin` & `PinValue`)

Data exchange is type-safe via `std::variant`. This avoids the need for hundreds of specific pin classes.

- **`PinValue`**: A variant holding `double`, `int`, `bool`, `vec3`, or `GeometryHandle`.
- **`GeometryHandle`**: A critical optimization. We do not pass heavy geometry data (MBs of vertices) through the graph. We pass a lightweight **Handle (EntityID)** referencing the `BodyComponent` that holds the actual B-Rep shape.

### The Connection (`Topology`)

OntoFlow follows a **Pull-Principle**. Connections are stored at the **Input**.

- An Input "knows" where its data comes from.
- An Output does not know who consumes its data.

<!-- end list -->

```cpp
struct Connection {
    EntityID targetNodeID;  // The source node (Provider)
    size_t targetPinIdx;    // The specific output index
};
```

## The Execution Model (The Physiology)

The engine separates the **Definition** of logic from the **Execution** of logic.

### The Node Registry (The DNA)

The `NodeRegistry` is a singleton factory containing `NodeDefinition` blueprints.

- **Input:** A `definitionID` (string).
- **Output:** A Lambda function (`ComputeFn`) representing the logic $f(x)$.

Example for a Box Node:

> **DefinitionID:** "GEOM_BOX"
> **Inputs:** Width, Height, Depth
> **Logic:** `Shape = OCCT::MakeBox(In[0], In[1], In[2])`

### The Graph Evaluator (The Pulse)

The evaluation strategy is **Recursive Pull with Change Detection**.

#### Algorithm: `Evaluate(Node N)`

1.  **Check Cache:** If `N` is not dirty, return immediately.
2.  **Pull Inputs:** Iterate over all Input Pins of `N`.
    - If Input has a **Connection**:
      - Recursively call `Evaluate(SourceNode)`.
      - **Change Detection:** Compare the current value in `InputPin` with the new value from `SourceNode.Output`.
      - If values differ: Update `InputPin` and mark `N` as **Dirty**.
    - If Input has **No Connection**:
      - Use the static value stored in the pin (Manual Override).
3.  **Compute:** If `N` is Dirty:
    - Lookup `ComputeFn` in Registry using `N.definitionID`.
    - Execute `ComputeFn(N)`.
    - Update `N.OutputPins`.
    - Set `N.isDirty = false`.

---

## Visual Representation

### Dependency Graph ($G$)

- **Nodes (A, B, C)** are Entities.
- **Edges** are defined inside `NodeComponent.inputs`.
- Data flows from Source (Left) to Sink (Right).

### The "Geometry Link"

Geometry nodes (like "Box") act as **Generators**.

1.  The Node Logic calls the Geometry Kernel (OCCT).
2.  It creates/updates a separate **`BodyComponent`** on the same Entity.
3.  The `NodeComponent` Output Pin outputs a `GeometryHandle` pointing to itself.
4.  Downstream nodes (e.g., "Boolean Cut") read this handle to access the topological shape.

---

## Use Cases & Scalability

### Hybrid Modeling

This architecture supports both **Static CAD** and **Dynamic Simulation**:

- **Static:** Inputs are constant values. Evaluator runs once.
- **Dynamic:** Inputs are Time-dependent functions (Sine Wave). Evaluator runs every frame.

### Extensibility

New functionality does not require changing the core engine structure.

1.  Define Inputs/Outputs.
2.  Write C++ Lambda (or Lua Script).
3.  Register in `NodeRegistry`.
