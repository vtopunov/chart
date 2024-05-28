#include "SampleApplication.h"

#include "shader_utils.h"
#include "system_utils.h"
#include "tga_utils.h"


namespace
{
    std::string concat(std::string_view left, std::string_view right)
    {
        std::string result;
        result.reserve(left.size() + right.size());
        result.append(left);
        result.append(right);
        return result;
    }

    bool loadTexture(GLuint texture, const std::string& path)
    {
        TGAImage img{};
        if (LoadTGAImageFromFile(path, &img))
        {
            if (img.width > 0 && img.height > 0)
            {
                LoadTextureFromTGAImage(texture, img);
                return true;
            }
        }

        return false;
    }

    class MultiTextureSample final : public SampleApplication
    {
    public:
        MultiTextureSample() : SampleApplication("MultiTexture", 1280, 720) {}

        bool initialize() override
        {
            {
                constexpr std::string_view vs =
                    R"(attribute vec2 a_position;
                attribute vec2 a_texCoord;
                varying vec2 v_texCoord;
                void main()
                {
                    gl_Position = vec4(a_position, 0.0, 1.0);
                    v_texCoord = a_texCoord;
                })";

                constexpr std::string_view fs =
                    R"(precision mediump float;
                varying vec2 v_texCoord;
                uniform sampler2D s_baseMap;
                uniform sampler2D s_lightMap;
                void main()
                {
                    vec4 baseColor;
                    vec4 lightColor;
    
                    baseColor = texture2D(s_baseMap, v_texCoord);
                    lightColor = texture2D(s_lightMap, v_texCoord);
                    gl_FragColor = baseColor * (lightColor + 0.25);
                })";

                mProgram = CompileProgram(vs, fs);
                if (!mProgram)
                {
                    return false;
                }
            }

            mPositionLoc = glGetAttribLocation(mProgram, "a_position");
            mTexCoordLoc = glGetAttribLocation(mProgram, "a_texCoord");

            {
                constexpr const char* textureNames[]
                {
                    "s_baseMap",
                    "s_lightMap"
                };
                static_assert(std::size(textureNames) == mNumberOfTextures);

                for (GLsizei i = 0; i < mNumberOfTextures; ++i)
                {
                    mTextureLocations[i] = glGetUniformLocation(mProgram, textureNames[i]);
                }
            }

            glGenTextures(mNumberOfTextures, std::data(mTextures));
            {
                constexpr const char* textureFiles[]
                {
                    "/basemap.tga",
                    "/lightmap.tga"
                };
                static_assert(std::size(textureFiles) == mNumberOfTextures);

                for (GLsizei i = 0; i < mNumberOfTextures; ++i)
                {
                    if (!loadTexture(mTextures[i], concat(angle::GetExecutableDirectory(), textureFiles[i])))
                    {
                        return false;
                    }
                }
            }

            return true;
        }

        void destroy() override
        {
            glDeleteProgram(mProgram);
            glDeleteTextures(mNumberOfTextures, std::data(mTextures));
        }

        void draw() override
        {
            constexpr GLfloat vertices[]
            {
                -0.5f, 0.5f,   // Position 0
                0.0f,  0.0f,   // TexCoord 0
                -0.5f, -0.5f,  // Position 1
                0.0f,  1.0f,   // TexCoord 1
                0.5f,  0.5f,   // Position 3
                1.0f,  0.0f,   // TexCoord 3
                0.5f,  -0.5f,  // Position 2
                1.0f,  1.0f    // TexCoord 2

            };
            constexpr GLubyte indices[] { 0, 1, 3, 0, 3, 2 };

            // Set the viewport
            glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

            // Clear the color buffer
            glClear(GL_COLOR_BUFFER_BIT);

            // Use the program object
            glUseProgram(mProgram);

            // Load the vertex position
            glVertexAttribPointer(mPositionLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), vertices);
            // Load the texture coordinate
            glVertexAttribPointer(mTexCoordLoc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
                                  vertices + 2);

            glEnableVertexAttribArray(mPositionLoc);
            glEnableVertexAttribArray(mTexCoordLoc);

            for (GLsizei i = 0; i < mNumberOfTextures; ++i)
            {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, mTextures[i]);
                glUniform1i(mTextureLocations[i], i);
            };

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);
        }

    private:
        // Handle to a program object
        GLuint mProgram{};

        // Attribute locations
        GLint mPositionLoc;
        GLint mTexCoordLoc;

        // Texture
        static constexpr GLsizei mNumberOfTextures{ 2 };
        std::array<GLint, mNumberOfTextures> mTextureLocations{};
        std::array<GLuint, mNumberOfTextures> mTextures{};
    };
}


int main(int argc, char** argv)
{
    MultiTextureSample app;
    return app.run();
}
