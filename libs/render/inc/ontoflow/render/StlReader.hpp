#pragma once

#include <ontoflow/domain/Types.hpp>
#include <string>

namespace of::render {

/// \brief Loader for ASCII and binary STL files into a TriMesh.
///        Designed as a stateless, reusable component.
class StlReader final {
   public:
    StlReader() = default;
    ~StlReader() = default;

    StlReader(const StlReader&) = default;
    StlReader(StlReader&&) noexcept = default;
    StlReader& operator=(const StlReader&) = default;
    StlReader& operator=(StlReader&&) noexcept = default;

    /// \brief Load an STL file (ASCII or binary) into a TriMesh.
    /// \param filePath Path to the STL file.
    /// \param outMesh  Target mesh. Will be overwritten on success.
    /// \return true on success, false on failure.
    [[nodiscard]] bool LoadFromFile(const std::string& filePath, domain::Mesh& outMesh) const;
};

}  // namespace of::render
