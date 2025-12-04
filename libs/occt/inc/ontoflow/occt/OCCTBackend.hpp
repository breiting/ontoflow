#pragma once
#include <memory>
#include <ontoflow/domain/IGeometryBackend.hpp>
#include <unordered_map>

// Forward declare OCCT types
class TopoDS_Shape;

namespace of::occt {

/// OCCT-based geometry backend.
/// Owns a collection of TopoDS_Shapes referenced by opaque handles.
class OCCTBackend : public domain::IGeometryBackend {
   public:
    OCCTBackend();
    ~OCCTBackend() override;

    domain::BackendShapeHandle CreateExtrudedBody(const domain::Polygon& profile, double height) override;
    domain::BackendShapeHandle CreateBox(double width, double length, double height) override;
    domain::BackendShapeHandle CreateCylinder(double radius, double height) override;

    bool ExportShapeToSTEP(domain::BackendShapeHandle handle, const std::string& filePath) const override;
    bool ExportShapeToSTL(domain::BackendShapeHandle handle, const std::string& filePath,
                          double deflection) const override;

    domain::Mesh GetMeshFromShape(domain::BackendShapeHandle handle) override;

   private:
    domain::BackendShapeHandle m_NextHandle{1};  // start with 1
    std::unordered_map<domain::BackendShapeHandle, std::unique_ptr<TopoDS_Shape>> m_Shapes;

    TopoDS_Shape* GetShape(domain::BackendShapeHandle handle) const;
    domain::BackendShapeHandle StoreShape(const TopoDS_Shape& shape);
};

}  // namespace of::occt
