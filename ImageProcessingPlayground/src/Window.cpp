#include "Window.h"

#include "Assert.h"

namespace playground {

    static void s_ErrorCallback(int error_code, const char* description)
    {
        std::cerr << "!! GLFW Error [" << error_code << "]: " << description << '\n';
    }

    static void s_OnWindowClose(GLFWwindow* window)
    {
        std::cout << "Event received: [OnWindowClose] \n";
        Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
        win->MarkToClose();
    }

    Window* Window::s_Instance = nullptr;
    Window* Window::Create(int width, int height, const std::string& title, bool fullscreen)
    {
        if (s_Instance != nullptr)
        {
            std::cout << "There is already an instance of Window\n";
        }
        else {
            std::cout << "Creating new window instance!\n";
            s_Instance = new Window(width, height, title, fullscreen);
        }
        return s_Instance;
    }

    Window::Window(int width, int height, const std::string& title, bool fullscreen)
        : m_width(width), m_height(height), m_title(title), m_fullscreen(fullscreen), m_isGLFWInitialized(false), m_markedToClose(false)
    {
        m_InitNativeWindow(width, height, title.c_str(), fullscreen);
    }

    Window::~Window()
    {
        ASSERT(m_NativeWin, "Native window is null")
        glfwDestroyWindow(m_NativeWin);
        glfwTerminate();
        s_Instance = nullptr;
    }

    void Window::m_InitNativeWindow(int width, int height, const std::string& title, bool fullscreen)
    {
        if (!m_isGLFWInitialized)
        {
            std::cout << "Initializing GLFW\n";
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
            int success = glfwInit();
            if (success == GLFW_FALSE)
            {
                std::cerr << "!! Error initializing GLFW\n";
                m_markedToClose = true;
                return;
            }
            glfwSetErrorCallback(s_ErrorCallback);
            m_isGLFWInitialized = true;
        }

        if (!fullscreen)
        {
            m_NativeWin = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        }
        else {
            ASSERT(false, "Fullscreen not yet implemented");
        }

        glfwMakeContextCurrent(m_NativeWin);
        int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        ASSERT(status, "Glad Loader failed!");

        const char* GPU_info = (const char*)glGetString(GL_RENDERER);
        std::cout << "Rendering hardware: " << GPU_info << '\n';

        glfwSwapInterval(1);

        ASSERT(m_NativeWin, "Native window could not be created!");
        glfwSetWindowUserPointer(m_NativeWin, this);

        // Set callbacks for native window
        glfwSetWindowCloseCallback(m_NativeWin, s_OnWindowClose);
    }

}