#include <glm/gtc/type_ptr.hpp>
#include <ontoflow/core/Logger.hpp> // Include Logger
#include <ontoflow/vis/TextRenderer2D.hpp>

namespace of::vis {

static const char* kTextVS = R"(
#version 410 core
layout(location=0) in vec2 aPos;
layout(location=1) in vec2 aUV;
layout(location=2) in vec3 aColor;

uniform mat4 uProj;

out vec2 vUV;
out vec3 vColor;

void main() {
    vUV = aUV;
    vColor = aColor;
    gl_Position = uProj * vec4(aPos, 0.0, 1.0);
}
)";

static const char* kTextFS = R"(
#version 410 core
in vec2 vUV;
in vec3 vColor;

out vec4 FragColor;

uniform sampler2D uTex;

void main() {
    float alpha = texture(uTex, vUV).r;
    if (alpha < 0.01)
        discard;
    FragColor = vec4(vColor, alpha);
}
)";

// -------------------------------------------------
// Internal helpers
// -------------------------------------------------
/**
 * \brief Creates and links the shader program for text rendering.
 * Uses hardcoded vertex and fragment shader sources.
 */
void TextRenderer2D::CreateShader() {
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &kTextVS, nullptr);
    glCompileShader(vs);

    int success;
    char infoLog[512];
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vs, 512, nullptr, infoLog);
        LOG(Error) << "TextRenderer2D Shader: Error compiling vertex shader: " << infoLog;
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &kTextFS, nullptr);
    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fs, 512, nullptr, infoLog);
        LOG(Error) << "TextRenderer2D Shader: Error compiling fragment shader: " << infoLog;
    }

    m_Shader = glCreateProgram();
    glAttachShader(m_Shader, vs);
    glAttachShader(m_Shader, fs);
    glLinkProgram(m_Shader);

    glGetProgramiv(m_Shader, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_Shader, 512, nullptr, infoLog);
        LOG(Error) << "TextRenderer2D Shader: Error linking shader program: " << infoLog;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
}

/**
 * \brief Creates and configures the OpenGL vertex buffer objects (VAO, VBO).
 */
void TextRenderer2D::CreateBuffers() {
    glGenVertexArrays(1, &m_Vao);
    glGenBuffers(1, &m_Vbo);

    glBindVertexArray(m_Vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);

    // Reserve some space – will be resized as needed
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(Vertex) * 1024,  // initial, will grow
                 nullptr, GL_DYNAMIC_DRAW);

    // layout(location=0) vec2 aPos;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));

    // layout(location=1) vec2 aUV;
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, u));

    // layout(location=2) vec3 aColor;
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, r));

    glBindVertexArray(0);
}

// -------------------------------------------------
// Public API
// -------------------------------------------------
/**
 * \brief Constructs a TextRenderer2D object.
 * Initializes internal OpenGL resource IDs to zero.
 */
TextRenderer2D::TextRenderer2D() : m_Atlas(nullptr), m_Vao(0), m_Vbo(0), m_Shader(0) {
    m_Vertices.reserve(2048); // Pre-allocate some capacity
}

/**
 * \brief Destructor. Deletes the associated OpenGL VAO, VBO, and Shader program.
 */
TextRenderer2D::~TextRenderer2D() {
    if (m_Shader) {
        glDeleteProgram(m_Shader);
        m_Shader = 0;
    }
    if (m_Vao) {
        glDeleteVertexArrays(1, &m_Vao);
        m_Vao = 0;
    }
    if (m_Vbo) {
        glDeleteBuffers(1, &m_Vbo);
        m_Vbo = 0;
    }
}

/**
 * \brief Initializes the text renderer with a glyph atlas and font metrics.
 * Must be called before any text rendering operations.
 * \param atlas A constant reference to the GlyphAtlas to use.
 * \param ascent The font's ascent metric.
 * \param descent The font's descent metric.
 * \param lineGap The font's line gap metric.
 */
void TextRenderer2D::Init(const GlyphAtlas& atlas, float ascent, float descent, float lineGap) {
    m_Atlas = &atlas;
    m_Ascent = ascent;
    m_Descent = descent;
    m_LineGap = lineGap;
    CreateShader();
    CreateBuffers();
}

/**
 * \brief Starts a new text batch.
 * Clears the internal CPU vertex buffer, preparing for new text strings.
 */
void TextRenderer2D::BeginBatch() {
    m_Vertices.clear();
}

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
void TextRenderer2D::AddText(const std::string& text, float x, float y, float scale, const glm::vec3& color) {
    if (!m_Atlas) {
        LOG(Error) << "TextRenderer2D: No glyph atlas initialized. Cannot add text.";
        return;
    }

    m_StartX = x;
    float penX = x;
    float baselineY = y;

    for (char c : text) {
        const auto* g = m_Atlas->GetGlyph(c);

        if (c == '\n') {
            penX = m_StartX;  // Reset X
            baselineY += (m_Ascent - m_Descent + m_LineGap) * scale;
            continue;
        }

        if (!g || g->size.x <= 0.0f || g->size.y <= 0.0f) {
            // Glyph exists, but is blank (e.g. space) or not found: still advance!
            penX += g ? (g->advance * scale) : (10.0f * scale);  // Fallback advance for unknown/blank glyphs
            continue;
        }

        float gw = g->size.x * scale;
        float gh = g->size.y * scale;

        // bearing (x0, y0) comes from stbtt_GetCodepointBitmapBox, relative to baseline
        float xpos = penX + g->bearing.x * scale;
        float ypos = baselineY + g->bearing.y * scale;

        float x0 = xpos;
        float y0 = ypos;
        float x1 = xpos + gw;
        float y1 = ypos + gh;

        float u0 = g->uv.x;
        float v0 = g->uv.y;
        float u1 = g->uv.z;
        float v1 = g->uv.w;

        // Two triangles (CCW) forming a quad
        // Triangle 1
        m_Vertices.push_back({x0, y0, u0, v0, color.r, color.g, color.b});
        m_Vertices.push_back({x1, y0, u1, v0, color.r, color.g, color.b});
        m_Vertices.push_back({x1, y1, u1, v1, color.r, color.g, color.b});
        // Triangle 2
        m_Vertices.push_back({x0, y0, u0, v0, color.r, color.g, color.b});
        m_Vertices.push_back({x1, y1, u1, v1, color.r, color.g, color.b});
        m_Vertices.push_back({x0, y1, u0, v1, color.r, color.g, color.b});

        penX += g->advance * scale;
    }
}

/**
 * \brief Uploads batched vertices to the GPU and draws them in a single draw call.
 * Requires the appropriate projection matrix.
 * \param proj The projection matrix (e.g., orthographic for screen space).
 */
void TextRenderer2D::Flush(const glm::mat4& proj) {
    if (!m_Atlas || m_Vertices.empty())
        return;

    glUseProgram(m_Shader);
    glUniformMatrix4fv(glGetUniformLocation(m_Shader, "uProj"), 1, GL_FALSE, glm::value_ptr(proj));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_Atlas->GetTexture());
    glUniform1i(glGetUniformLocation(m_Shader, "uTex"), 0);

    glBindVertexArray(m_Vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);

    // Upload full batch in one go
    glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(Vertex), m_Vertices.data(), GL_DYNAMIC_DRAW);

    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_Vertices.size()));

    glBindVertexArray(0);
}

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
void TextRenderer2D::DrawText(const std::string& text, float x, float y, float scale, const glm::vec3& color,
                              const glm::mat4& proj) {
    BeginBatch();
    AddText(text, x, y, scale, color);
    Flush(proj);
}

}  // namespace of::vis
