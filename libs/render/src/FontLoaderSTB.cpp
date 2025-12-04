#include <fstream>
#include <ontoflow/core/Logger.hpp> // Include Logger
#include <ontoflow/render/FontLoaderSTB.hpp>
#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#include "stb_truetype.h"
#pragma clang diagnostic pop

namespace of::render {

/**
 * \brief Internal helper to load font data using stb_truetype.
 * \param data Pointer to the font data.
 * \param pixelHeight Desired pixel height.
 * \return True on success, false on failure.
 */
bool FontLoaderSTB::LoadFromSTB(const uint8_t* data, float pixelHeight) {
    stbtt_fontinfo font{};
    if (!stbtt_InitFont(&font, data, 0)) {
        LOG(Error) << "FontLoaderSTB: Failed to initialize font.";
        return false;
    }

    float scale = stbtt_ScaleForPixelHeight(&font, pixelHeight);

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);
    m_Ascent = ascent * scale;
    m_Descent = descent * scale;
    m_LineGap = lineGap * scale;

    m_Glyphs.clear();

    for (char c = 32; c < 127; ++c) { // Load ASCII printable characters
        int x0, y0, x1, y1;
        stbtt_GetCodepointBitmapBox(&font, c, scale, scale, &x0, &y0, &x1, &y1);

        int w = x1 - x0;
        int h = y1 - y0;
        if (w <= 0 || h <= 0) {
            // Glyph exists, but is blank (e.g. space) or has no bitmap
            // Still process metrics if possible
            int advanceWidth, leftSideBearing;
            stbtt_GetCodepointHMetrics(&font, c, &advanceWidth, &leftSideBearing);
            
            FontGlyph g;
            g.size = {0.0f, 0.0f}; // No visual size
            g.bearing = {static_cast<float>(x0), static_cast<float>(y0)};
            g.advance = advanceWidth * scale;
            g.bitmap.clear(); // No bitmap data
            m_Glyphs[c] = std::move(g);
            continue;
        }

        int bw, bh;
        unsigned char* bmp = stbtt_GetCodepointBitmap(&font, 0.0f, scale, c, &bw, &bh, nullptr, nullptr);
        if (!bmp) {
            LOG(Warn) << "FontLoaderSTB: Failed to get bitmap for character: " << c;
            continue;
        }

        // Sanity: w/h should match bw/bh
        if (bw != w || bh != h) {
            // Adjust if stb_truetype returned different dimensions
            w = bw;
            h = bh;
            LOG(Warn) << "FontLoaderSTB: Bitmap dimensions mismatch for char " << c << ". Adjusting.";
        }

        FontGlyph g;
        g.size = {static_cast<float>(w), static_cast<float>(h)};
        g.bearing = {static_cast<float>(x0), static_cast<float>(y0)};  // Box relative to baseline

        int advanceWidth, leftSideBearing;
        stbtt_GetCodepointHMetrics(&font, c, &advanceWidth, &leftSideBearing);
        g.advance = advanceWidth * scale;

        g.bitmap.assign(bmp, bmp + (static_cast<size_t>(w) * bh));
        stbtt_FreeBitmap(bmp, nullptr);

        m_Glyphs[c] = std::move(g);
    }

    return true;
}

/**
 * \brief Loads TTF font data from a memory buffer.
 * \param data Pointer to the TTF font data in memory.
 * \param pixelHeight The desired pixel height of the font.
 * \return True if the font was loaded successfully, false otherwise.
 */
bool FontLoaderSTB::LoadTTFMemory(const uint8_t* data, float pixelHeight) {
    if (!data) {
        LOG(Error) << "FontLoaderSTB: Invalid memory block provided.";
        return false;
    }
    return LoadFromSTB(data, pixelHeight);
}

/**
 * \brief Loads TTF font data from a file.
 * \param fileName The path to the TTF file.
 * \param pixelHeight The desired pixel height of the font.
 * \return True if the font was loaded successfully, false otherwise.
 */
bool FontLoaderSTB::LoadTTF(const std::string& fileName, float pixelHeight) {
    std::ifstream file(fileName, std::ios::binary);
    if (!file) {
        LOG(Error) << "FontLoaderSTB: Could not open TTF file: " << fileName;
        return false;
    }

    // Read file into buffer
    std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    return LoadFromSTB(buffer.data(), pixelHeight);
}

/**
 * \brief Retrieves the `FontGlyph` for a given character.
 * \param c The character to retrieve.
 * \return A pointer to the `FontGlyph` if found, `nullptr` otherwise.
 */
const FontGlyph* FontLoaderSTB::GetGlyph(char c) const {
    auto it = m_Glyphs.find(c);
    return (it != m_Glyphs.end()) ? &it->second : nullptr;
}

}  // namespace of::render
