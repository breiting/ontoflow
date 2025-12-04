#pragma once

#include <string>

namespace of::vis {

/**
 * \brief Manages an OpenGL 2D texture.
 *
 * This class handles loading image data using `stb_image`, creating an
 * OpenGL texture object, and providing methods for binding and unbinding it.
 */
class Texture {
   public:
    /**
     * \brief Loads an image from the specified path and creates an OpenGL texture.
     * \param path The file path to the image.
     */
    Texture(const std::string& path);

    /**
     * \brief Destructor. Deletes the OpenGL texture object.
     */
    ~Texture();

    /**
     * \brief Binds the texture to a texture unit.
     * \param texLoc The uniform location of the sampler in the shader.
     */
    void Bind(unsigned int texLoc) const;

    /**
     * \brief Unbinds the currently active texture.
     */
    void Unbind() const;

    /**
     * \brief Returns the width of the texture in pixels.
     */
    int GetWidth() const {
        return m_Width;
    }
    /**
     * \brief Returns the height of the texture in pixels.
     */
    int GetHeight() const {
        return m_Height;
    }
    /**
     * \brief Calculates and returns the aspect ratio (width / height) of the texture.
     */
    float GetAspectRatio() const {
        return static_cast<float>(m_Width) / static_cast<float>(m_Height);
    }

   private:
    unsigned int m_TextureId; ///< The OpenGL texture ID.
    int m_Width, m_Height, m_Channels; ///< Width, height, and number of channels of the loaded image.
};
}  // namespace of::vis
