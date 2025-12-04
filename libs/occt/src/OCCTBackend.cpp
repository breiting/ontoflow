#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Triangle.hxx>
#include <Poly_Triangulation.hxx>
#include <STEPControl_StepModelType.hxx>
#include <STEPControl_Writer.hxx>
#include <StlAPI_Writer.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <ontoflow/core/Logger.hpp>
#include <ontoflow/occt/OCCTBackend.hpp>

using namespace of::domain;

namespace of::occt {

OCCTBackend::OCCTBackend() = default;

OCCTBackend::~OCCTBackend() = default; // Unique_ptr handles cleanup

BackendShapeHandle OCCTBackend::StoreShape(const TopoDS_Shape& shape) {
    BackendShapeHandle handle = m_NextHandle++;
    m_Shapes[handle] = std::make_unique<TopoDS_Shape>(shape);
    return handle;
}

TopoDS_Shape* OCCTBackend::GetShape(BackendShapeHandle handle) const {
    auto it = m_Shapes.find(handle);
    if (it == m_Shapes.end()) {
        return nullptr;
    }
    return it->second.get();
}

BackendShapeHandle OCCTBackend::CreateBox(double width, double length, double height) {
    BRepPrimAPI_MakeBox box(width, length, height);
    box.Build();
    if (!box.IsDone()) {
        LOG(Error) << "CreateBox: Failed to create box.";
        return 0;
    }
    return StoreShape(box.Shape());
}

BackendShapeHandle OCCTBackend::CreateCylinder(double radius, double height) {
    BRepPrimAPI_MakeCylinder cyl(radius, height);
    cyl.Build();
    if (!cyl.IsDone()) {
        LOG(Error) << "CreateCylinder: Failed to create cylinder.";
        return 0;
    }
    return StoreShape(cyl.Shape());
}

BackendShapeHandle OCCTBackend::CreateExtrudedBody(const Polygon& profile, double height) {
    if (profile.vertices.size() < 3) {
        LOG(Error) << "CreateExtrudedBody: Profile has fewer than 3 vertices.";
        return 0;
    }

    BRepBuilderAPI_MakeWire wireBuilder;

    const auto& v = profile.vertices;
    const std::size_t n = v.size();
    for (std::size_t i = 0; i < n; ++i) {
        const auto& a = v[i];
        const auto& b = v[(i + 1) % n];
        BRepBuilderAPI_MakeEdge edge(gp_Pnt(a.x, a.y, a.z), gp_Pnt(b.x, b.y, b.z));
        if (!edge.IsDone()) {
            LOG(Error) << "CreateExtrudedBody: Failed to create edge " << i;
            return 0;
        }
        wireBuilder.Add(edge.Edge());
    }

    if (!wireBuilder.IsDone()) {
        LOG(Error) << "CreateExtrudedBody: Wire builder failed.";
        return 0;
    }

    TopoDS_Wire wire = wireBuilder.Wire();
    BRepBuilderAPI_MakeFace faceMaker(wire, true);
    if (!faceMaker.IsDone()) {
        LOG(Error) << "CreateExtrudedBody: Face maker failed.";
        return 0;
    }
    TopoDS_Face face = faceMaker.Face();

    gp_Vec dir(0.0, 0.0, height);
    BRepPrimAPI_MakePrism prism(face, dir);
    if (!prism.IsDone()) {
        LOG(Error) << "CreateExtrudedBody: Prism (extrusion) failed.";
        return 0;
    }
    TopoDS_Shape body = prism.Shape();

    return StoreShape(body);
}

bool OCCTBackend::ExportShapeToSTEP(BackendShapeHandle handle, const std::string& filePath) const {
    TopoDS_Shape* shape = GetShape(handle);
    if (!shape) {
        LOG(Error) << "ExportSTEP: Invalid handle " << handle;
        return false;
    }

    if (shape->IsNull()) {
        LOG(Error) << "ExportSTEP: Shape is null for handle " << handle;
        return false;
    }

    STEPControl_Writer writer;
    IFSelect_ReturnStatus status = writer.Transfer(*shape, STEPControl_AsIs);
    if (status != IFSelect_RetDone) {
        LOG(Error) << "ExportSTEP: Transfer failed with status " << (int)status;
        return false;
    }

    status = writer.Write(filePath.c_str());
    if (status != IFSelect_RetDone) {
        LOG(Error) << "ExportSTEP: Write failed with status " << (int)status;
        return false;
    }

    return true;
}

bool OCCTBackend::ExportShapeToSTL(BackendShapeHandle handle, const std::string& filePath, double deflection) const {
    TopoDS_Shape* shape = GetShape(handle);
    if (!shape)
        return false;

    if (shape->IsNull())
        return false;

    BRepMesh_IncrementalMesh mesher(*shape, deflection);
    StlAPI_Writer stlWriter;
    stlWriter.Write(*shape, filePath.c_str());
    return true;
}

domain::Mesh OCCTBackend::GetMeshFromShape(BackendShapeHandle handle) {
    domain::Mesh mesh;
    TopoDS_Shape* shape = GetShape(handle);
    if (!shape || shape->IsNull()) {
        LOG(Warn) << "GetMeshFromShape: Invalid handle or null shape.";
        return mesh;
    }

    const double deflection = 0.5; // Deflection parameter controls tesselation quality
    BRepMesh_IncrementalMesh mesher(*shape, deflection, false);

    TopExp_Explorer faceExp(*shape, TopAbs_FACE);
    for (; faceExp.More(); faceExp.Next()) {
        TopoDS_Face face = TopoDS::Face(faceExp.Current());

        TopLoc_Location loc;
        Handle(Poly_Triangulation) tri = BRep_Tool::Triangulation(face, loc);
        if (tri.IsNull())
            continue;

        gp_Trsf trsf = loc.Transformation();
        bool flipWinding = (face.Orientation() == TopAbs_REVERSED);

        std::size_t baseIndex = mesh.vertices.size();

        const int nbV = tri->NbNodes();
        for (int i = 1; i <= nbV; ++i) {
            gp_Pnt p = tri->Node(i).Transformed(trsf);
            Vertex v;
            v.SetPosition(glm::vec3(static_cast<float>(p.X()), static_cast<float>(p.Y()), static_cast<float>(p.Z())));
            v.SetColor(glm::vec3(0.7f, 0.7f, 0.8f));
            mesh.vertices.push_back(v);
        }

        const int nbT = tri->NbTriangles();
        for (int i = 1; i <= nbT; ++i) {
            Poly_Triangle t = tri->Triangle(i);
            int i1, i2, i3;
            t.Get(i1, i2, i3);

            if (flipWinding)
                std::swap(i2, i3);

            mesh.indices.push_back(static_cast<uint32_t>(baseIndex + i1 - 1));
            mesh.indices.push_back(static_cast<uint32_t>(baseIndex + i2 - 1));
            mesh.indices.push_back(static_cast<uint32_t>(baseIndex + i3 - 1));
        }
    }

    // Basic normal calculation if missing
    // (For now, assuming we might want to calculate normals here or in rendering system.
    //  Doing it here ensures data quality on domain level.)
    if (!mesh.vertices.empty() && mesh.indices.size() % 3 == 0) {
        std::vector<glm::vec3> normals(mesh.vertices.size(), glm::vec3(0.0f));
        for (std::size_t i = 0; i < mesh.indices.size(); i += 3) {
            uint32_t ia = mesh.indices[i + 0];
            uint32_t ib = mesh.indices[i + 1];
            uint32_t ic = mesh.indices[i + 2];

            glm::vec3 a = mesh.vertices[ia].GetPosition();
            glm::vec3 b = mesh.vertices[ib].GetPosition();
            glm::vec3 c = mesh.vertices[ic].GetPosition();

            glm::vec3 n = glm::normalize(glm::cross(b - a, c - a));
            normals[ia] += n;
            normals[ib] += n;
            normals[ic] += n;
        }
        for (std::size_t i = 0; i < mesh.vertices.size(); ++i) {
             // Check length to avoid NaN
             if (glm::length(normals[i]) > 0.0001f)
                mesh.vertices[i].SetNormal(glm::normalize(normals[i]));
        }
    }

    return mesh;
}

}  // namespace of::occt