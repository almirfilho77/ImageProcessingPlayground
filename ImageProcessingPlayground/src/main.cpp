#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include "Window.h"
#include "Assert.h"

constexpr uint32_t WIN_WIDTH = 640;
constexpr uint32_t WIN_HEIGHT = 480;
constexpr const char* WIN_TITLE = "OpenCV Playground window";

static bool created = false;
static uint32_t vertex_shader;
static uint32_t fragment_shader;
static uint32_t program;
void CreateCanvas()
{
    if (!created)
    {
        const char* vertex_shader_text =
            "#version 430 core\n"
            "layout(location = 0) in vec3 a_Position;\n"
            "out vec4 v_Color;\n"
            "void main()\n"
            "{\n"
            "    gl_Position = a_Position;\n"
            "    v_Color = vec4(a_Position, 1.0);\n"
            "}\n";

        const char* fragment_shader_text =
            "#version 430 core\n"
            "in vec4 v_Color;\n"
            "void main()\n"
            "{\n"
            "    color = v_Color;\n"
            "}\n";

        vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
        glCompileShader(vertex_shader);

        fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
        glCompileShader(fragment_shader);

        program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);

        created = true;
    }
}

void Render()
{
    auto vertices = CreateInfraToRender();

    vpos_location = glGetAttribLocation(program, "a_Position");
    vcol_location = glGetAttribLocation(program, "vCol");

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

    glfwGetFramebufferSize(m_NativeWin, &m_width, &m_height);

    glViewport(0, 0, m_width, m_height);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(m_NativeWin);
    glfwPollEvents();
}

uint32_t image_vertex_buffer;
uint32_t image_index_buffer;
void RenderImage(const cv::Mat& image)
{
    // Create a vertex buffer
    static float image_canvas[4 * 3] = {
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
    };
    glGenBuffers(1, &image_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, image_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(image_canvas), image_canvas, GL_STATIC_DRAW);

    // Create index buffer
    static uint32_t image_indexes[6] = {
        0, 1, 2, 0, 3, 2,
    };
    glGenBuffers(1, &image_index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, image_index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6, image_indexes, GL_STATIC_DRAW);

    // Load a texture
}


int main(int argc, char** argv)
{
    playground::Window* win = playground::Window::Create(WIN_WIDTH, WIN_HEIGHT, WIN_TITLE, false);
    CreateCanvas();

    cv::Mat image;
    image = cv::imread("football.png", cv::IMREAD_COLOR); // Read the file
    if (image.empty()) // Check for invalid input
    {
        std::cout << "Could not open or find the image" << std::endl;
        return -1;
    }

    /*
    * cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE); // Create a window for display.
    * cv::imshow("Display window", image); // Show our image inside it.
    * cv::waitKey(0); // Wait for a keystroke in the window
    */

    int width, height;
    while (!win->IsMarkedToClose())
    {
        RenderImage(image);
    }
    ASSERT(win != 0, "Window is null");
    return 0;
}