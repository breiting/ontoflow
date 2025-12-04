#pragma once

#include <glad.h>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <string>
#include <vector>

#include "GlyphAtlas.hpp"

namespace of::render {

/**
 * \brief Renders 2D text using a batched approach and a glyph atlas.
 *
 * This class handles the creation of OpenGL resources (VAO, VBO, Shader)
 * for text rendering and provides methods to add text to a batch buffer,
 * upload it to the GPU, and draw it in a single call.
 */
class TextRenderer2D {
   public:
    /**
     * \brief Constructs a TextRenderer2D object.
     * Initializes internal OpenGL resource IDs to zero.
     */
    TextRenderer2D();

    /**
     * \brief Destructor. Deletes the associated OpenGL VAO, VBO, and Shader program.
     */
    ~TextRenderer2D();

    /**
     * \brief Initializes the text renderer with a glyph atlas and font metrics.
     * Must be called before any text rendering operations.
     * \param atlas A constant reference to the GlyphAtlas to use.
     * \param ascent The font's ascent metric.
     * \param descent The font's descent metric.
     * \param lineGap The font's line gap metric.
     */
    void Init(const GlyphAtlas& atlas, float ascent, float descent, float lineGap);

    /**
     * \brief Starts a new text batch.
     * Clears the internal CPU vertex buffer, preparing for new text strings.
     */
    void BeginBatch();

    /**
     * \brief Adds one text string to the current batch.
     * The text is buffered on the CPU and will be rendered on `Flush()`.
     * (x, y) is the baseline position in screen space.
     * \param text The string to add.
     * \param x X-coordinate of the baseline start in screen space.
     * \param y Y-coordinate of the baseline start in screen space.
     * \param scale Scaling factor for the text.
     * \param color The RGB color of the text.
     */
    void AddText(const std::string& text, float x, float y, float scale, const glm::vec3& color);

    /**
     * \brief Uploads batched vertices to the GPU and draws them in a single draw call.
     * Requires the appropriate projection matrix.
     * \param proj The projection matrix (e.g., orthographic for screen space).
     */
    void Flush(const glm::mat4& proj);

    /**
     * \brief Convenience wrapper for one-off text drawing.
     * Internally calls `BeginBatch()`, `AddText()`, and `Flush()`.
     * \param text The string to draw.
     * \param x X-coordinate of the baseline start in screen space.
     * \param y Y-coordinate of the baseline start in screen space.
     * \param scale Scaling factor for the text.
     * \param color The RGB color of the text.
     * \param proj The projection matrix.
     */
    void DrawText(const std::string& text, float x, float y, float scale, const glm::vec3& color,
                  const glm::mat4& proj);

   private:
    /**
     * \brief Structure representing a single vertex for text rendering.
     */
    struct Vertex {
        float x, y;     ///< Position of the vertex.
        float u, v;     ///< UV coordinates for the glyph atlas.
        float r, g, b;  ///< RGB color of the vertex.
    };

    const GlyphAtlas* m_Atlas = nullptr; ///< Pointer to the glyph atlas used for texturing.
    GLuint m_Vao = 0; ///< OpenGL Vertex Array Object ID.
    GLuint m_Vbo = 0; ///< OpenGL Vertex Buffer Object ID.
    GLuint m_Shader = 0; ///< OpenGL Shader Program ID.

    std::vector<Vertex> m_Vertices;  ///< CPU-side buffer for batched text vertices.

    /**
     * \brief Creates and links the shader program for text rendering.
     * Uses hardcoded vertex and fragment shader sources.
     */
    void CreateShader();

    /**
     * \brief Creates and configures the OpenGL vertex buffer objects (VAO, VBO).
     */
    void CreateBuffers();

    float m_Ascent = 0.0f;  ///< Font ascent metric.
    float m_Descent = 0.0f; ///< Font descent metric.
    float m_LineGap = 0.0f; ///< Font line gap metric.
    float m_StartX = 0.0f;  ///< Starting X position for multiline text.
};

}  // namespace of::render
