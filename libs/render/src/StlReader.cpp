#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <limits>
#include <ontoflow/core/Logger.hpp> // Include Logger
#include <ontoflow/render/StlReader.hpp>
#include <sstream>
#include <string>

using namespace of::domain;

namespace of::render {

namespace {

// STL binary layout constants (see STL spec)
constexpr std::streamoff kStlBinaryHeaderSize = 80;
constexpr std::streamoff kStlBinaryTriangleSize = 50;  // 12 bytes normal + 36 bytes verts + 2 bytes attribute

enum class StlFormat {
    Binary,
    Ascii
};

/**
 * \brief Gets the size of a file.
 * \param file The ifstream object.
 * \return The size of the file in bytes.
 */
[[nodiscard]] std::streamoff GetFileSize(std::ifstream& file) {
    const auto currentPos = file.tellg();
    file.seekg(0, std::ios::end);
    const auto size = file.tellg();
    file.seekg(currentPos);
    return size;
}

/// \brief Tries to decide whether a file is binary or ASCII STL.
///
/// Heuristic:
/// 1. Compare file size with (80 + 4 + n * 50) for binary format.
/// 2. If it doesn't match, treat as ASCII.
/// 3. If header starts with "solid" and doesn't match binary size, it's ASCII.
///    (This covers 99% of cases reasonably well.)
/// \param file The ifstream object of the STL file.
/// \return The detected StlFormat (Binary or Ascii).
[[nodiscard]] StlFormat DetectFormat(std::ifstream& file) {
    file.clear();
    file.seekg(0, std::ios::beg);

    const std::streamoff fileSize = GetFileSize(file);

    // Read header + triangle count
    char header[static_cast<std::size_t>(kStlBinaryHeaderSize)]{};
    file.read(header, kStlBinaryHeaderSize);

    std::uint32_t triangleCount = 0;
    file.read(reinterpret_cast<char*>(&triangleCount), sizeof(triangleCount));

    if (!file.good()) {
        file.clear();
        file.seekg(0, std::ios::beg);
        return StlFormat::Ascii; // If cannot read enough, probably ASCII
    }

    const std::streamoff expectedSize = kStlBinaryHeaderSize + static_cast<std::streamoff>(sizeof(triangleCount)) +
                                        static_cast<std::streamoff>(triangleCount) * kStlBinaryTriangleSize;

    file.clear();
    file.seekg(0, std::ios::beg);

    if (expectedSize == fileSize) {
        return StlFormat::Binary;
    }

    // Heuristic: if header begins with "solid" and does not exactly
    // match the binary size, then ASCII is very likely.
    const std::string headerStr(header, header + 5);
    if (headerStr == "solid") {
        return StlFormat::Ascii;
    }

    // Fallback: ASCII, if we don't trust the size.
    return StlFormat::Ascii;
}

/**
 * \brief Loads a binary STL file into a Mesh object.
 * \param file The ifstream object, positioned at the beginning of the file.
 * \param mesh The Mesh object to populate.
 * \return True on success, false on failure.
 */
[[nodiscard]] bool LoadBinaryStl(std::ifstream& file, Mesh& mesh) {
    mesh.Clear();

    file.clear();
    file.seekg(0, std::ios::beg);

    // Header
    char header[static_cast<std::size_t>(kStlBinaryHeaderSize)]{};
    file.read(header, kStlBinaryHeaderSize);

    // Triangle count
    std::uint32_t triangleCount = 0;
    file.read(reinterpret_cast<char*>(&triangleCount), sizeof(triangleCount));
    if (!file.good()) {
        LOG(Error) << "StlReader: Failed to read triangle count in binary STL.";
        return false;
    }

    // Reserve memory: each triangle -> 3 vertices, 3 indices
    mesh.vertices.reserve(static_cast<std::size_t>(triangleCount) * 3);
    mesh.indices.reserve(static_cast<std::size_t>(triangleCount) * 3);

    for (std::uint32_t i = 0; i < triangleCount; ++i) {
        float normal[3]{};
        float v1[3]{};
        float v2[3]{};
        float v3[3]{};
        std::uint16_t attributeByteCount = 0;

        file.read(reinterpret_cast<char*>(normal), sizeof(normal));
        file.read(reinterpret_cast<char*>(v1), sizeof(v1));
        file.read(reinterpret_cast<char*>(v2), sizeof(v2));
        file.read(reinterpret_cast<char*>(v3), sizeof(v3));
        file.read(reinterpret_cast<char*>(&attributeByteCount), sizeof(attributeByteCount));

        if (!file.good()) {
            LOG(Error) << "StlReader: Unexpected EOF while reading binary STL. Triangle " << i << "/" << triangleCount;
            return false;
        }

        const glm::vec3 normalVec{normal[0], normal[1], normal[2]};

        const auto addVertex = [&mesh, &normalVec](const float v[3]) {
            Vertex vertex{};
            vertex.SetPosition(glm::vec3{v[0], v[1], v[2]});
            vertex.SetNormal(normalVec);

            mesh.vertices.push_back(vertex);
            mesh.indices.push_back(static_cast<std::uint32_t>(mesh.vertices.size() - 1));
        };

        addVertex(v1);
        addVertex(v2);
        addVertex(v3);
    }

    return true;
}

/**
 * \brief Loads an ASCII STL file into a Mesh object.
 * \param file The ifstream object, positioned at the beginning of the file.
 * \param mesh The Mesh object to populate.
 * \return True on success, false on failure.
 */
[[nodiscard]] bool LoadAsciiStl(std::ifstream& file, Mesh& mesh) {
    mesh.Clear();

    file.clear();
    file.seekg(0, std::ios::beg);

    std::string line;
    glm::vec3 currentNormal{0.0f, 0.0f, 0.0f};
    bool inFacet = false;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;
        ss >> token;

        if (token == "facet") {
            // facet normal nx ny nz
            std::string normalKeyword;
            ss >> normalKeyword >> currentNormal.x >> currentNormal.y >> currentNormal.z;
            inFacet = true;
        } else if (token == "vertex" && inFacet) {
            glm::vec3 position{};
            ss >> position.x >> position.y >> position.z;

            Vertex vertex{};
            vertex.SetPosition(position);
            vertex.SetNormal(currentNormal);

            mesh.vertices.push_back(vertex);
            mesh.indices.push_back(static_cast<std::uint32_t>(mesh.vertices.size() - 1));
        } else if (token == "endfacet") {
            // robustness: ignore if not exactly 3 vertices
            inFacet = false;
        }
    }

    if (!file.eof() && file.fail()) {
        LOG(Error) << "StlReader: Error while reading ASCII STL (badbit or failbit set).";
        return false;
    }

    return !mesh.vertices.empty();
}

}  // namespace

/**
 * \brief Load an STL file (ASCII or binary) into a TriMesh.
 * \param filePath Path to the STL file.
 * \param outMesh  Target mesh. Will be overwritten on success.
 * \return true on success, false on failure.
 */
[[nodiscard]] bool StlReader::LoadFromFile(const std::string& filePath, Mesh& outMesh) const {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        LOG(Error) << "StlReader: Could not open file: " << filePath;
        return false;
    }

    const StlFormat format = DetectFormat(file);

    switch (format) {
        case StlFormat::Binary:
            LOG(Info) << "StlReader: Loading binary STL from " << filePath;
            return LoadBinaryStl(file, outMesh);
        case StlFormat::Ascii:
            LOG(Info) << "StlReader: Loading ASCII STL from " << filePath;
            return LoadAsciiStl(file, outMesh);
        default:
            LOG(Error) << "StlReader: Unknown STL format for file: " << filePath;
            return false;
    }
}

}  // namespace of::render
