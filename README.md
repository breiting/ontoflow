# 🌌 OntoFlow

_An Ontological Engine for Computational Emergence and Parametric Design_

## 🚀 Introduction

OntoFlow is a new kind of computational design system. Not a CAD tool. Not a simulation engine. It is an **ontological machine** — a system that allows structures, geometry, and behavior to _emerge_ from the interaction of simple nodes, laws, and agents.

Instead of drawing shapes or issuing commands, the user **defines a world**:

- _Matter_ — geometric primitives or imported topology
- _Agents_ — active components with state and behavior
- _Laws_ — constraints, functions, and transforms
- _Concepts_ — abstract data, time, groups, signals

By connecting these building blocks inside a **Node Graph**, users construct _systems that generate form_, _systems that simulate_, _systems that evolve_. OntoFlow is inspired by cybernetics, complexity theory, and the elegance of functional flow-based programming.

It brings together:

- the rigor of B-Rep geometry (OpenCascade)
- the clarity of an ECS (Entity Component System)
- the expressiveness of Lua scripting
- the immediacy of visual nodes

All of this is wrapped in a minimal, fast, open-source C++17 codebase.

## 🎓 Research Abstract

OntoFlow is a data-oriented, ontological simulation environment designed to bridge the gap between deterministic B-Rep geometry and stochastic Agent-Based Modeling (ABM). Unlike traditional CAD systems, which rely on static history trees, OntoFlow implements a dynamic ECS architecture to model a “computational ontology.”

The system abstracts the design space into four metaphysical categories:

- **Matter**: Topological manifolds or geometric primitives
- **Agents**: Stateful automata with heuristic behaviors
- **Laws**: Global or local constraints (logic, functions, equations)
- **Concepts**: Abstract data structures

A flow-based programming interface allows users to build directed acyclic graphs (DAGs) where nodes represent transformations of state rather than explicit modeling commands.

OntoFlow combines two runtimes:

- **Passive Evaluation Engine** — parametric topology, pull-based DAG evaluation
- **Active Simulation Loop** — agents, solvers, constraints, push-based dynamics

This duality enables _inverse design_, where shapes emerge from interacting components rather than explicit modeling.

## 🧩 Philosophy

OntoFlow follows the principle:

> **From Simplicity to Complexity**  
> Complex phenomena emerge from simple interactions.

### Design Principles

- **Minimalistic** — no bloat, no excessive UI, no corporate CAD complexity
- **Modular** — everything is a component, everything is replaceable
- **Composable** — build systems by wiring blocks
- **Deterministic or Dynamic** — parametric modeling _and_ simulation
- **Scriptable** — Lua for automation, behavior, meta-programming
- **Open Source** — transparent, hackable, extensible

OntoFlow is for:

- makers, hardware hackers
- computational designers
- academic researchers
- robotics & simulation developers
- people who want clarity and control — not black boxes

## 🧱 Architecture Overview

```
┌──────────────────────────────┐
│          OntoFlow            │
├──────────────────────────────┤
│  C++17 Core (Data-Oriented)  │
│  Entity-Component-System     │
│  Node Graph & Evaluator      │
│  OpenCascade Geometry Layer  │
│  ImGui / ImNodes UI          │
│  Lua Scripting Engine        │
└──────────────────────────────┘
```

## Core Modules

| Module              | Description                                          |
| ------------------- | ---------------------------------------------------- |
| **ECS**             | Bare-metal, data-oriented entity-component system    |
| **Node Graph**      | Nodes, Pins, DAG evaluation                          |
| **GraphEvaluator**  | Executes nodes, propagates values, triggers geometry |
| **GeometryGateway** | Stateless interface to OpenCascade                   |
| **Renderer**        | Minimal OpenGL renderer for preview                  |
| **Lua Runtime**     | Behavior, rules, agents                              |
| **UI Layer**        | ImGui + ImNodes for visual graph editing             |

## 📦 Dependencies

OntoFlow requires and is based on:

- **C++17**
- **OpenCascade 7.9+**
- **GLFW**
- **ImGui + ImNodes**
- **Lua 5.4**
- **glm**
- **CMake 3.31+**

## 🔧 Build Instructions

```bash
git clone https://github.com/breiting/ontoflow.git
cd ontoflow
mkdir build && cd build
cmake ..
make -j
```

## 🔭 Roadmap

OntoFlow just started as an idea and a concept in mind. It is currently under heavy development, and will get features along its development roadmap.

## 🤝 Contributing

OntoFlow is intentionally open, transparent, and modular.  
Contributions are welcome — from research ideas to code to testing.

## 📜 License

OntoFlow is released under the **MIT License**.  
You are free to use, modify, and distribute it, both commercially and non-commercially.

## ⭐ Philosophy Reminder

> OntoFlow is not a product.  
> It is a tool — a conceptual machine.  
> A space for computational creativity.  
> A playground for systems thinkers.  
> A bridge between code and geometry, physics and design, logic and emergence.

## ⭐ Support the project

If you like this idea — star ⭐ the repo — it helps big time! More stars → more contributors → more speed.

Welcome to the new frontier of computational design.
