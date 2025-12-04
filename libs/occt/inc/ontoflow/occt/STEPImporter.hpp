#pragma once
#include <ontoflow/domain/Registry.hpp>
#include <string>

class TopoDS_Shape;  // forward vom OCCT

namespace of::occt {

class STEPImporter {
   public:
    STEPImporter() = default;

    /// @return Entity with MeshComponent or INVALID_ENTITY and modified registry
    of::domain::Entity Load(const std::string& filename, of::domain::Registry& registry);

   private:
    void TriangulateShape(const TopoDS_Shape& shape, of::domain::MeshComponent& outMesh);
    void ExtractTopology(const TopoDS_Shape& shape, of::domain::Registry& registry);
};

}  // namespace of::occt
