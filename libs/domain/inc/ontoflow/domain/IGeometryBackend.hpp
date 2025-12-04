#pragma once
#include <ontoflow/domain/Components.hpp>
#include <ontoflow/domain/Types.hpp>
#include <string>

namespace of::domain {

/// Interface for a geometry backend (OCCT or other).
class IGeometryBackend {
   public:
    virtual ~IGeometryBackend() = default;

    /// Create a solid body by extruding a planar polygon along its normal.
    virtual BackendShapeHandle CreateExtrudedBody(const Polygon& profile, double height) = 0;

    /// Create a solid box.
    virtual BackendShapeHandle CreateBox(double width, double length, double height) = 0;

    /// Create a solid cylinder.
    /// \param radius Radius of the cylinder.
    /// \param height Height of the cylinder.
    /// \return Handle to the created shape.
    virtual BackendShapeHandle CreateCylinder(double radius, double height) = 0;

    /// Export an existing backend shape as STEP.
    virtual bool ExportShapeToSTEP(BackendShapeHandle handle, const std::string& filePath) const = 0;

    /// Export as STL with a meshing tolerance.
    virtual bool ExportShapeToSTL(BackendShapeHandle handle, const std::string& filePath, double deflection) const = 0;

    /// Tesselate a shape into a mesh.
    virtual Mesh GetMeshFromShape(BackendShapeHandle handle) = 0;
};

}  // namespace of::domain
