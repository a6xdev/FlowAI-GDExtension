# Architecture & Concepts

> **Note:** This page is a work in progress. More details and examples will be added over time.

## How it works

FlowAI is built around a two-layer navigation hierarchy managed by a single `FlowAIManager`:

- **Macro Graph**: a high-level graph of sectors covering the whole map
- **Micro Graph**: a lightweight, temporary graph built on demand for each path request

---

## Macro Graph

The macro graph divides the world into sectors. Each sector represents a region of the map, and two sectors are only connected if there is at least one real pathnode link crossing the boundary between them, not just because they're geometrically adjacent.

This matters. A purely grid-based macro would connect sectors that have no actual walkable path between them, causing agents to get stuck. FlowAI avoids this by deriving macro connections from the micro data during the bake process.

When an NPC at point **A** wants to reach point **B**, the manager runs a fast A* over the macro graph to find a corridor of sectors:

```
[Sector 1 (A)] → [Sector 2] → [Sector 3] → [Sector 4 (B)]
```

This corridor typically spans 3–6 sectors, which is a fraction of the full map.

---

## Micro Graph

Instead of keeping a giant graph active for the entire map, FlowAI builds a **temporary local graph** covering only the sectors in the corridor. This graph is created, queried, and discarded entirely within a single `request_path()` call.

Because each agent gets its own isolated graph, there is no shared mutable state between agents, making the system safe for many simultaneous requests without any coordination overhead.

### Navigation Layers

Each `FlowAIPathnode` has a `navigation_layers` bitmask, similar to Godot's collision layers. Agents only traverse nodes that share at least one layer with them. This lets you define separate paths for pedestrians, vehicles, or any custom category without duplicating the graph.

---

## Bake

The bake process runs inside the Godot Editor. The `FlowAIManager` scans all `FlowAIPathnode` nodes in the scene, assigns each one to a sector based on its world position and the configured `sector_size`, and detects which pathnode links cross sector boundaries.

The result is saved as a lightweight `.tres` resource (`FlowAIBakeData`) containing:
- which pathnodes belong to each sector
- which sectors are connected to which, and through which navigation layers

At runtime, the manager reads this file to reconstruct the macro graph. It never re-scans the scene tree or recalculates sector assignments.

---

## Why hierarchical?

In open-world navigation, running a full A* across thousands of nodes for hundreds of agents simultaneously isn't viable. The hierarchical approach solves this in two steps:

1. The macro graph eliminates ~90% of the map before any detailed search begins
2. The micro graph only processes the nodes that actually matter for that specific request

The memory cost per agent scales with corridor size, not map size. And because the micro graph is local and temporary, there's no activation/deactivation bookkeeping, no shared state, and no coordination between agents.

---

[Watch a demo on YouTube](https://www.youtube.com/watch?v=dj7vYgY9aCo)