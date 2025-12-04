#include <filesystem>
#include <ontoflow/core/Logger.hpp> // Include Logger
#include <ontoflow/render/FontSystem.hpp>

#include "roboto_regular.h"

namespace of::render {

/**
 * \brief Loads font data from a memory buffer and builds its glyph atlas.
 * \param data Pointer to the TTF font data in memory.
 * \param pixelHeight The desired pixel height of the font.
 * \return True if the font and atlas were loaded successfully, false otherwise.
 */
bool FontSystem::LoadFromMemory(const uint8_t* data, float pixelHeight) {
    if (!m_Loader.LoadTTFMemory(data, pixelHeight)) {
        LOG(Error) << "FontSystem: Failed to load font from memory.";
        return false;
    }
    if (!m_Atlas.BuildFrom(m_Loader)) {
        LOG(Error) << "FontSystem: Failed to build glyph atlas from memory font.";
        return false;
    }
    LOG(Info) << "FontSystem: Loaded font from memory (pixel height: " << pixelHeight << ").";
    return true;
}

/**
 * \brief Attempts to find a font file in common system font locations.
 * This is a utility function to help locate TTF files.
 * \param fileName The name of the font file (e.g., "Arial.ttf").
 * \return The full path to the font file if found, an empty string otherwise.
 */
std::string FontSystem::FindFont(const std::string& fileName) {
    const std::vector<std::string> locations = {"/System/Library/Fonts/", "/Library/Fonts/",
                                                std::string(getenv("HOME")) + "/Library/Fonts/"};

    for (const auto& dir : locations) {
        std::filesystem::path p = dir + fileName;
        if (std::filesystem::exists(p))
            return p.string();
    }
    LOG(Warn) << "FontSystem: Font file '" << fileName << "' not found in standard locations.";
    return "";
}

/**
 * \brief Loads font data from a file and builds its glyph atlas.
 * \param path The path to the TTF file.
 * \param pixelHeight The desired pixel height of the font.
 * \return True if the font and atlas were loaded successfully, false otherwise.
 */
bool FontSystem::LoadFromFile(const std::string& path, float pixelHeight) {
    if (!m_Loader.LoadTTF(path, pixelHeight)) {
        LOG(Error) << "FontSystem: Failed to load TTF file: " << path;
        return false;
    }
    if (!m_Atlas.BuildFrom(m_Loader)) {  // or BuildMSDFCore(...) later
        LOG(Error) << "FontSystem: Failed to build atlas for file: " << path;
        return false;
    }
    LOG(Info) << "FontSystem: Loaded font from: " << path << " (pixel height: " << pixelHeight << ").";
    return true;
}

/**
 * \brief Loads a default embedded font (Roboto Regular) and builds its glyph atlas.
 * \return True if the default font and atlas were loaded successfully, false otherwise.
 */
bool FontSystem::LoadDefaultFont() {
    constexpr float PIXEL_HEIGHT = 48.0f;
    return LoadFromMemory(g_RobotoRegular, PIXEL_HEIGHT);
}

}  // namespace of::render
