#pragma once
#include <glad.h>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <unordered_map>

#include "FontLoaderSTB.hpp"

namespace of::render {

/**
 * \brief Manages an OpenGL texture atlas containing rendered glyphs.
 *
 * This class builds a single texture from individual glyph bitmaps provided
 * by `FontLoaderSTB` and provides UV coordinates for each glyph within that atlas.
 */
class GlyphAtlas {
   public:
    /**
     * \brief Stores information about a glyph within the texture atlas.
     */
    struct GlyphUV {
        glm::vec2 size;     ///< Pixel size of glyph.
        glm::vec2 bearing;  ///< Offset from baseline.
        float advance;      ///< Horizontal distance to advance to the next glyph.
        glm::vec4 uv;       ///< UV rectangle in atlas [u0, v0, u1, v1].
    };

    /**
     * \brief Constructs a GlyphAtlas.
     */
    GlyphAtlas() = default;

    /**
     * \brief Destructor. Deletes the OpenGL texture associated with the atlas.
     */
    ~GlyphAtlas();

    /**
     * \brief Builds the glyph atlas texture from a loaded font.
     * Generates an OpenGL texture from the font's glyph bitmaps.
     * \param font A reference to the FontLoaderSTB instance containing glyph data.
     * \return True if the atlas was built successfully, false otherwise.
     */
    bool BuildFrom(const FontLoaderSTB& font);

    /**
     * \brief Returns the OpenGL texture ID of the glyph atlas.
     * \return The GLuint texture ID.
     */
    GLuint GetTexture() const {
        return m_Texture;
    }

    /**
     * \brief Retrieves `GlyphUV` information for a given character.
     * \param c The character to retrieve.
     * \return A pointer to the `GlyphUV` data if found, `nullptr` otherwise.
     */
    const GlyphUV* GetGlyph(char c) const;

    /**
     * \brief Returns the font's ascent metric, copied from the FontLoader.
     * \return Ascent in pixels.
     */
    float GetAscent() const {
        return m_Ascent;
    }

   private:
    GLuint m_Texture = 0; ///< The OpenGL texture ID for the glyph atlas.
    std::unordered_map<char, GlyphUV> m_Glyphs; ///< Map of glyph UV data, keyed by character.
    float m_Ascent{0.0}; ///< Font ascent metric.
};

}  // namespace of::render
