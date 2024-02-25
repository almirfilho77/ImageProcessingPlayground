#include <iostream>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <windows.h>
//#include <GLFW/glfw3.h>

#include "Window.h"
#include "Assert.h"

constexpr uint32_t WIN_WIDTH = 640;
constexpr uint32_t WIN_HEIGHT = 480;
constexpr const char* WIN_TITLE = "OpenCV Playground window";

// ---------------- OpenGL Error handling ---------------- //
void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);

#ifdef _DEBUG

#define GLCallVoid(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__), "Assertion failed!")

#define GLCall(x) [&](){\
    GLClearError();\
    auto retval = x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__), "Assertion failed!")\
    return retval;\
    }()
#else

#define GlCallVoid(x) x
#define GlCall(x) x

#endif

void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        std::cout << "OpenGL Error " << error << '\n';
        return false;
    }
    return true;
}

// ---------------- OpenGL Error handling ---------------- //

// ---------------- Shader creation ---------------- //
static bool solidQuadShaderCreated = false;
static uint32_t solidQuadVAO;
static uint32_t solidQuadVertexShaderID;
static uint32_t solidQuadFragmentShaderID;
static uint32_t solidQuadProgram;
void CreateSolidColorShader()
{
    if (!solidQuadShaderCreated)
    {
        GLCallVoid(glCreateVertexArrays(1, &solidQuadVAO));
        GLCallVoid(glBindVertexArray(solidQuadVAO));

        const char* solidColorVertexShader = R"(
            #version 430 core
            layout(location = 0) in vec3 a_Position;
            out vec3 v_Position;
            void main()
            {
                v_Position = a_Position;
                gl_Position = vec4(a_Position, 1.0);
            }
        )";

        const char* solidColorFragmentShader = R"(
            #version 430 core
            layout(location = 0) out vec4 color;
            in vec3 v_Position;
            void main()
            {
                color = vec4(v_Position * 0.5 + 0.5, 1.0);
            }
        )";

        solidQuadVertexShaderID = GLCall(glCreateShader(GL_VERTEX_SHADER));
        GLCallVoid(glShaderSource(solidQuadVertexShaderID, 1, &solidColorVertexShader, NULL));
        GLCallVoid(glCompileShader(solidQuadVertexShaderID));
        int success;
        glGetShaderiv(solidQuadVertexShaderID, GL_COMPILE_STATUS, &success);
        ASSERT(success, "Vertex Shader compilation failed!");

        solidQuadFragmentShaderID = GLCall(glCreateShader(GL_FRAGMENT_SHADER));
        GLCallVoid(glShaderSource(solidQuadFragmentShaderID, 1, &solidColorFragmentShader, NULL));
        GLCallVoid(glCompileShader(solidQuadFragmentShaderID));
        glGetShaderiv(solidQuadFragmentShaderID, GL_COMPILE_STATUS, &success);
        ASSERT(success, "Fragment Shader compilation failed!");

        solidQuadProgram = GLCall(glCreateProgram());
        GLCallVoid(glAttachShader(solidQuadProgram, solidQuadVertexShaderID));
        GLCallVoid(glAttachShader(solidQuadProgram, solidQuadFragmentShaderID));
        GLCallVoid(glLinkProgram(solidQuadProgram));
        glGetProgramiv(solidQuadProgram, GL_VALIDATE_STATUS, &success);
        ASSERT(success, "Shader linkage failed!");

        // Unbind shader
        GLCallVoid(glUseProgram(0));

        solidQuadShaderCreated = true;
    }
}

static bool imageShaderCreated = false;
static uint32_t imageVAO;
static uint32_t imageVertexShaderID;
static uint32_t imageFragmentShaderID;
static uint32_t imageProgram;
void CreateTextureShader()
{
    if (!imageShaderCreated)
    {
        GLCallVoid(glCreateVertexArrays(1, &imageVAO));
        GLCallVoid(glBindVertexArray(imageVAO));

        const char* imageVertexShader = R"(
            #version 430 core
            layout(location = 0) in vec3 a_Position;
            layout(location = 1) in vec2 a_texCoord;
            out vec2 v_texCoord;
            void main()
            {
                gl_Position = vec4(a_Position, 1.0);
                v_texCoord = a_texCoord;
            }
        )";

        const char* imageFragmentShader = R"(
            #version 430 core
            layout (location = 0) out vec4 color;
            in vec2 v_texCoord;
            uniform sampler2D u_Texture;
            void main()
            {
                vec4 texColor = texture2D(u_Texture, v_texCoord);
                color = texColor;
            }
        )";

        imageVertexShaderID = GLCall(glCreateShader(GL_VERTEX_SHADER));
        GLCallVoid(glShaderSource(imageVertexShaderID, 1, &imageVertexShader, NULL));
        GLCallVoid(glCompileShader(imageVertexShaderID));
        int success;
        glGetShaderiv(solidQuadVertexShaderID, GL_COMPILE_STATUS, &success);
        ASSERT(success, "Vertex Shader compilation failed!");

        imageFragmentShaderID = GLCall(glCreateShader(GL_FRAGMENT_SHADER));
        GLCallVoid(glShaderSource(imageFragmentShaderID, 1, &imageFragmentShader, NULL));
        GLCallVoid(glCompileShader(imageFragmentShaderID));
        glGetShaderiv(imageFragmentShaderID, GL_COMPILE_STATUS, &success);
        ASSERT(success, "Fragment Shader compilation failed!");

        imageProgram = GLCall(glCreateProgram());
        GLCallVoid(glAttachShader(imageProgram, imageVertexShaderID));
        GLCallVoid(glAttachShader(imageProgram, imageFragmentShaderID));
        GLCallVoid(glLinkProgram(imageProgram));
        glGetProgramiv(imageProgram, GL_VALIDATE_STATUS, &success);
        ASSERT(success, "Shader linkage failed!");

        // Unbind shader
        GLCallVoid(glUseProgram(0));

        imageShaderCreated = true;
    }
}

// ---------------- Shader creation ---------------- //


// ---------------- Canvas creation ---------------- //
static bool solidColorObjectCreated = false;
static uint32_t solidQuadVBO_ID;
static uint32_t solidQuadIBO_ID;
void CreateSolidQuadCanvas()
{
    if (!solidColorObjectCreated)
    {
        CreateSolidColorShader();

        static float solidQuad[4 * 3] = {
            -1.0f, -1.0f, 0.0f,
             1.0f, -1.0f, 0.0f,
             1.0f,  1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f,
        };
        GLCallVoid(glGenBuffers(1, &solidQuadVBO_ID));
        GLCallVoid(glBindBuffer(GL_ARRAY_BUFFER, solidQuadVBO_ID));
        GLCallVoid(glBufferData(GL_ARRAY_BUFFER, sizeof(solidQuad), solidQuad, GL_STATIC_DRAW));

        // a_Position
        GLCallVoid(glEnableVertexAttribArray(0));
        GLCallVoid(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr));

        // Create index buffer
        static uint32_t indices[6] = {
            0, 1, 2, 0, 2, 3,
        };
        GLCallVoid(glGenBuffers(1, &solidQuadIBO_ID));
        GLCallVoid(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, solidQuadIBO_ID));
        GLCallVoid(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));

        GLCallVoid(glBindVertexArray(0));
        GLCallVoid(glDisableVertexAttribArray(0));
        GLCallVoid(glBindBuffer(GL_ARRAY_BUFFER, 0));
        GLCallVoid(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

        solidColorObjectCreated = true;
    }
}

static bool imageCreated = false;
static uint32_t imageVBO_ID;
static uint32_t imageIBO_ID;
void CreateImageCanvas()
{
    if (!imageCreated)
    {
        CreateTextureShader();

        static float imageQuad[4 * 5] = {
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        };
        GLCallVoid(glGenBuffers(1, &imageVBO_ID));
        GLCallVoid(glBindBuffer(GL_ARRAY_BUFFER, imageVBO_ID));
        GLCallVoid(glBufferData(GL_ARRAY_BUFFER, sizeof(imageQuad), imageQuad, GL_STATIC_DRAW));

        // a_Position
        GLCallVoid(glEnableVertexAttribArray(0));
        GLCallVoid(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr));

        // a_TexCoord
        GLCallVoid(glEnableVertexAttribArray(1));
        GLCallVoid(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))));

        // Create index buffer
        static uint32_t indices[6] = {
            0, 1, 2, 0, 2, 3,
        };
        GLCallVoid(glGenBuffers(1, &imageIBO_ID));
        GLCallVoid(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, imageIBO_ID));
        GLCallVoid(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));

        GLCallVoid(glBindVertexArray(0));
        GLCallVoid(glDisableVertexAttribArray(0));
        GLCallVoid(glDisableVertexAttribArray(1));
        GLCallVoid(glBindBuffer(GL_ARRAY_BUFFER, 0));
        GLCallVoid(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

        imageCreated = true;
    }
}

// ---------------- Canvas creation ---------------- //

// ---------------- Graphic Object creation ---------------- //
void RenderSolidColorQuad()
{
    CreateSolidQuadCanvas();
    GLCallVoid(glBindVertexArray(solidQuadVAO));
    GLCallVoid(glUseProgram(solidQuadProgram));
    
    int width, height;
    glfwGetFramebufferSize(playground::Window::Get()->GetNativeWin(), &width, &height);
    GLCallVoid(glViewport(0, 0, width, height));
    GLCallVoid(glClearColor(0.8f, 0.2f, 0.2f, 1.0f));
    GLCallVoid(glClear(GL_COLOR_BUFFER_BIT));

    GLCallVoid(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

    glfwSwapBuffers(playground::Window::Get()->GetNativeWin());
    glfwPollEvents();
}

static bool textureUploaded = false;
static uint32_t imageTextureID;
void UploadTexture(unsigned char *imageData, int channels, int width, int height)
{
    if (!textureUploaded)
    {
        GLenum internalFormat = 0, dataFormat = 0;
        std::cout << "#channels: " << channels << '\n';
        switch (channels)
        {
        case 3:
            internalFormat = GL_RGB8;
            dataFormat = GL_RGB;
            break;

        case 4:
            internalFormat = GL_RGBA8;
            dataFormat = GL_RGBA;
            break;

        default:
            ASSERT(false, "Format not supported!");
            
        }
        // Upload the texture
        GLCallVoid(glCreateTextures(GL_TEXTURE_2D, 1, &imageTextureID));
        GLCallVoid(glTextureStorage2D(imageTextureID, 1, internalFormat, width, height))

        GLCallVoid(glTextureParameteri(imageTextureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GLCallVoid(glTextureParameteri(imageTextureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GLCallVoid(glTextureParameteri(imageTextureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GLCallVoid(glTextureParameteri(imageTextureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

        GLCallVoid(glTextureSubImage2D(imageTextureID, 0, 0, 0, width, height,
            dataFormat, GL_UNSIGNED_BYTE, imageData));

        GLCallVoid(glBindTexture(GL_TEXTURE_2D, 0));

        textureUploaded = true;
    }
}

void RenderImage(const cv::Mat& image)
{
    cv::Mat flippedImage;
    cv::flip(image, flippedImage, 0);
    cv::cvtColor(flippedImage, flippedImage, cv::COLOR_BGR2RGB);
    UploadTexture(flippedImage.data, image.channels(), image.cols, image.rows);
    GLCallVoid(glBindVertexArray(imageVAO));
    GLCallVoid(glUseProgram(imageProgram));

    int width, height;
    glfwGetFramebufferSize(playground::Window::Get()->GetNativeWin(), &width, &height);
    GLCallVoid(glViewport(0, 0, width, height));
    GLCallVoid(glClearColor(0.8f, 0.2f, 0.2f, 1.0f));
    GLCallVoid(glClear(GL_COLOR_BUFFER_BIT));

    GLCallVoid(glActiveTexture(GL_TEXTURE0));
    GLCallVoid(glBindTexture(GL_TEXTURE_2D, imageTextureID));
    int textureUniformLocation = GLCall(glGetUniformLocation(imageProgram, "u_Texture"));
    ASSERT(textureUniformLocation != -1, "Could not find textureLocation");
    GLCallVoid(glUniform1i(textureUniformLocation, 0));
    GLCallVoid(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

    glfwSwapBuffers(playground::Window::Get()->GetNativeWin());
    glfwPollEvents();
}

// ---------------- Graphic Object creation ---------------- //

int main(int argc, char** argv)
{
    playground::Window* win = playground::Window::Create(WIN_WIDTH, WIN_HEIGHT, WIN_TITLE, false);

    //cv::Mat image;
    //image = cv::imread("football.png", cv::IMREAD_COLOR); // Read the file
    //if (image.empty()) // Check for invalid input
    //{
    //    std::cout << "Could not open or find the image" << std::endl;
    //    return -1;
    //}

#ifdef WIN32
    _putenv_s("OPENCV_FFMPEG_CAPTURE_OPTIONS", "rtsp_transport;udp");
#else
    setenv("OPENCV_FFMPEG_CAPTURE_OPTIONS", "rtsp_transport;udp", 1);
#endif
    int frameNum = -1;
    const std::string streamSource = "rtsp://admin:loco6005@192.168.1.76:554/onvif1";
    cv::VideoCapture ipCamStream(streamSource, cv::CAP_FFMPEG);
    if (!ipCamStream.isOpened())
    {
        std::cerr << "!! Could not open the video stream [" << streamSource << "]\n";
        return -1;
    }
    cv::Size videoFrameSize = cv::Size((int)ipCamStream.get(cv::CAP_PROP_FRAME_WIDTH), (int)ipCamStream.get(cv::CAP_PROP_FRAME_HEIGHT));
    std::cout << "Frame width [" << videoFrameSize.width << "] / height [" << videoFrameSize.height << "]\n";

    CreateImageCanvas();

    cv::Mat currentFrame;

    while (!win->IsMarkedToClose())
    {
        ipCamStream >> currentFrame;
        if (currentFrame.empty())
        {
            std::cout << "End of stream!\n";
            break;
        }
        textureUploaded = false;
        ++frameNum;
        std::cout << "Execute RenderImage on frame [" << frameNum << "]\n";
        RenderImage(currentFrame);
        //RenderSolidColorQuad();
    }
    ASSERT(win != 0, "Window is null");
    return 0;
}