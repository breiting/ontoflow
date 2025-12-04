#pragma once
#include <cstdint>
#include <glm/vec2.hpp>
#include <unordered_map>
#include <vector>
#include <string>

namespace of::render {

/**
 * \brief Represents a single font glyph's bitmap data and metrics.
 */
struct FontGlyph {
    glm::vec2 size;               ///< width/height in pixels of the bitmap.
    glm::vec2 bearing;            ///< Offset from baseline to the top-left of the glyph bitmap.
    float advance;                ///< Horizontal distance to advance to the next glyph.
    std::vector<uint8_t> bitmap;  ///< Grayscale alpha bitmap data for the glyph.
};

/**
 * \brief Loads TrueType Font (TTF) files using the stb_truetype library.
 *
 * This class handles parsing TTF data from memory or file, extracts glyph
 * bitmaps and metrics for ASCII characters (32-126), and provides access
 * to individual glyphs and font-wide metrics like ascent, descent, and line gap.
 */
class FontLoaderSTB {
   public:
    /**
     * \brief Loads TTF font data from a memory buffer.
     * \param data Pointer to the TTF font data in memory.
     * \param pixelHeight The desired pixel height of the font.
     * \return True if the font was loaded successfully, false otherwise.
     */
    bool LoadTTFMemory(const uint8_t* data, float pixelHeight);

    /**
     * \brief Loads TTF font data from a file.
     * \param fileName The path to the TTF file.
     * \param pixelHeight The desired pixel height of the font.
     * \return True if the font was loaded successfully, false otherwise.
     */
    bool LoadTTF(const std::string& fileName, float pixelHeight);

    /**
     * \brief Retrieves the `FontGlyph` for a given character.
     * \param c The character to retrieve.
     * \return A pointer to the `FontGlyph` if found, `nullptr` otherwise.
     */
    const FontGlyph* GetGlyph(char c) const;

    /**
     * \brief Returns the font's ascent metric (distance from baseline to top of highest glyph).
     * \return Ascent in pixels.
     */
    float GetAscent() const {
        return m_Ascent;
    }
    /**
     * \brief Returns the font's descent metric (distance from baseline to bottom of lowest glyph).
     * \return Descent in pixels.
     */
    float GetDescent() const {
        return m_Descent;
    }
    /**
     * \brief Returns the recommended vertical distance between two lines of text.
     * \return Line gap in pixels.
     */
    float GetLineGap() const {
        return m_LineGap;
    }

   private:
    /**
     * \brief Internal helper to load font data using stb_truetype.
     * \param data Pointer to the font data.
     * \param pixelHeight Desired pixel height.
     * \return True on success, false on failure.
     */
    bool LoadFromSTB(const unsigned char* data, float pixelHeight);

   private:
    std::unordered_map<char, FontGlyph> m_Glyphs; ///< Map of loaded glyphs, keyed by character.
    float m_Ascent = 0.f;   ///< Font ascent metric.
    float m_Descent = 0.f;  ///< Font descent metric.
    float m_LineGap = 0.f;  ///< Font line gap metric.
};

}  // namespace of::render
