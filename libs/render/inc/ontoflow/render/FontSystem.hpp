#pragma once
#include "FontLoaderSTB.hpp"
#include "GlyphAtlas.hpp"
#include <string>
#include <cstdint> // For uint8_t

namespace of::render {

/**
 * \brief Manages font loading and glyph atlas generation for text rendering.
 *
 * This class acts as a high-level interface for setting up a font,
 * loading it from various sources (memory, file, default embedded),
 * and preparing it for use by `TextRenderer2D` through a `GlyphAtlas`.
 */
class FontSystem {
   public:
    /**
     * \brief Loads font data from a memory buffer and builds its glyph atlas.
     * \param data Pointer to the TTF font data in memory.
     * \param pixelHeight The desired pixel height of the font.
     * \return True if the font and atlas were loaded successfully, false otherwise.
     */
    bool LoadFromMemory(const uint8_t* data, float pixelHeight);

    /**
     * \brief Loads font data from a file and builds its glyph atlas.
     * \param path The path to the TTF file.
     * \param pixelHeight The desired pixel height of the font.
     * \return True if the font and atlas were loaded successfully, false otherwise.
     */
    bool LoadFromFile(const std::string& path, float pixelHeight);

    /**
     * \brief Loads a default embedded font (Roboto Regular) and builds its glyph atlas.
     * \return True if the default font and atlas were loaded successfully, false otherwise.
     */
    bool LoadDefaultFont();

    /**
     * \brief Attempts to find a font file in common system font locations.
     * This is a utility function to help locate TTF files.
     * \param fileName The name of the font file (e.g., "Arial.ttf").
     * \return The full path to the font file if found, an empty string otherwise.
     */
    static std::string FindFont(const std::string& fileName);

    /**
     * \brief Returns a constant reference to the generated glyph atlas.
     * \return A const reference to the GlyphAtlas object.
     */
    const GlyphAtlas& GetAtlas() const {
        return m_Atlas;
    }

    /**
     * \brief Returns the font's ascent metric.
     * \return Ascent in pixels.
     */
    float GetAscent() const {
        return m_Loader.GetAscent();
    }
    /**
     * \brief Returns the font's descent metric.
     * \return Descent in pixels.
     */
    float GetDescent() const {
        return m_Loader.GetDescent();
    }
    /**
     * \brief Returns the font's line gap metric.
     * \return Line gap in pixels.
     */
    float GetLineGap() const {
        return m_Loader.GetLineGap();
    }

   private:
    FontLoaderSTB m_Loader; ///< The font loader responsible for parsing TTF data.
    GlyphAtlas m_Atlas;     ///< The glyph atlas storing rendered glyphs as a texture.
};

}  // namespace of::render
