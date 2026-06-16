# Flow AI Architecture

> ALERT: This is a temporary page. It will be improved in thefuture.

The system is separed in 3 main layers manager by a native way.

FlowAIManager:
- MacroGraph (Zones/Sectors)
- MicroGraph Pool (Active local nodes)

## MacroGraph (High-Level Graph)

Define sectors in the city (ex: traffic areas, highway , urban centers).

Use: When a NPC in the point *A* wanna go to point *B*, the FlowAIManager run a quick A* only for 5 or 6 steps in the macro graph to define a path that the NPC will walk through: ``[Sector 1 (A) -> Sector 2 -> Sector 3 -> Sector 4 (B)]``

## Micro Graph (Low-Level Graph)

Detailed data of paths for pedestrians an vehicles. This Micro Graph only is processed if the sector is active (close to the player's camera)

### Custom Layers:
 Is like godot collision layer configuration.

## Plugin Classes

FlowAIPathnode:
```js
class FlowAIPathNode : public Node3D {
    GDCLASS(FlowAIPathNode, Node3D);
public:
    uint32_t sector_id;
    uint32_t context_layers;  // Bitmask (Pedestrian, Vehicle, Crosswalk, etc.)
    float corridor_width;
    PackedInt32Array links;   // Connected Nodes ID
};
```

FlowAISector: Is a data structure.
```js
struct FlowAISector {
    uint32_t id;
    Vector3 center_position;
    std::vector<FlowAIPathNode*> micro_nodes;
    std::vector<uint32_t> neighbor_sectors;   // macro sectors neighbors
    bool is_active;                           // if its far from camera, become false
};
```

## Bake FlowAI

In the baking process, the FlowAIManager will create a dynamic grid to separate each sector based in ``FlowAIManager.sector_size``.