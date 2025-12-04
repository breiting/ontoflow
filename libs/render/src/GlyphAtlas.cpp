#include <ontoflow/render/GlyphAtlas.hpp>
#include <vector>

namespace of::render {

/**
 * \brief Destructor. Deletes the OpenGL texture associated with the atlas.
 */
GlyphAtlas::~GlyphAtlas() {
    if (m_Texture) {
        glDeleteTextures(1, &m_Texture);
        m_Texture = 0;
    }
}

/**
 * \brief Builds the glyph atlas texture from a loaded font.
 * Generates an OpenGL texture from the font's glyph bitmaps.
 * \param font A reference to the FontLoaderSTB instance containing glyph data.
 * \return True if the atlas was built successfully, false otherwise.
 */
bool GlyphAtlas::BuildFrom(const FontLoaderSTB& font) {
    const int atlasSize = 2048; // Hardcoded atlas size
    const int padding = 2; // Padding between glyphs in the atlas

    std::vector<unsigned char> atlas(atlasSize * atlasSize, 0); // Initialize with 0 (transparent)

    int penX = padding;
    int penY = padding;
    int rowHeight = 0;

    m_Glyphs.clear();
    m_Ascent = font.GetAscent();

    // Iterate through ASCII printable characters (32 to 126)
    for (char c = 32; c < 127; ++c) {
        const FontGlyph* g = font.GetGlyph(c);
        if (!g || g->size.x == 0 || g->size.y == 0) // Skip glyphs without bitmap data
            continue;

        int gw = static_cast<int>(g->size.x);
        int gh = static_cast<int>(g->size.y);

        // Check if current row is full, move to next row
        if (penX + gw + padding > atlasSize) {
            penX = padding;
            penY += rowHeight + padding;
            rowHeight = 0; // Reset row height for new row
        }
        // Check if atlas is full
        if (penY + gh + padding > atlasSize) {
            // For MVP, if atlas is full, we stop. This might mean not all glyphs are included.
            break;
        }

        // Copy glyph bitmap data into the atlas texture
        for (int y = 0; y < gh; ++y) {
            for (int x = 0; x < gw; ++x) {
                // Bounds check for safety, though should be covered by atlasSize checks
                if ((penY + y) < atlasSize && (penX + x) < atlasSize) {
                    atlas[(static_cast<size_t>(penY) + y) * atlasSize + (static_cast<size_t>(penX) + x)] = g->bitmap[static_cast<size_t>(y) * gw + x];
                }
            }
        }

        // Store UV coordinates and metrics for the glyph
        GlyphUV info;
        info.size = g->size;
        info.bearing = g->bearing;
        info.advance = g->advance;

        info.uv = glm::vec4(static_cast<float>(penX) / atlasSize, static_cast<float>(penY) / atlasSize,
                            static_cast<float>(penX + gw) / atlasSize, static_cast<float>(penY + gh) / atlasSize);

        m_Glyphs[c] = info;

        penX += gw + padding; // Advance pen position for next glyph
        if (gh > rowHeight)
            rowHeight = gh; // Update row height
    }

    // Delete existing texture if any
    if (m_Texture) {
        glDeleteTextures(1, &m_Texture);
        m_Texture = 0;
    }

    // Generate and bind new texture
    glGenTextures(1, &m_Texture);
    glBindTexture(GL_TEXTURE_2D, m_Texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

    // Upload atlas data to GPU
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, atlasSize, atlasSize, 0, GL_RED, GL_UNSIGNED_BYTE, atlas.data());

    // Set texture parameters for linear filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return true;
}

/**
 * \brief Retrieves `GlyphUV` information for a given character.
 * \param c The character to retrieve.
 * \return A pointer to the `GlyphUV` data if found, `nullptr` otherwise.
 */
const GlyphAtlas::GlyphUV* GlyphAtlas::GetGlyph(char c) const {
    auto it = m_Glyphs.find(c);
    return (it != m_Glyphs.end()) ? &it->second : nullptr;
}

}  // namespace of::render
