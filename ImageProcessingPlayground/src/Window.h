#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <iostream>
#include <string>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace playground {
    class Window {
    public:
        static Window* Create(int width, int height, const std::string& title, bool fullscreen = false);
        ~Window();
        static Window* Get() { return s_Instance; }

        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }
        const std::string& GetTitle() const { return m_title; }
        bool GetFullscreen() const { return m_fullscreen; }

        void MarkToClose() { m_markedToClose = true; }
        bool IsMarkedToClose() const { return m_markedToClose; }

        GLFWwindow* GetNativeWin() const { return m_NativeWin; }

    private:
        Window(int width, int height, const std::string& title, bool fullscreen);
        void m_InitNativeWindow(int width, int height, const std::string& title, bool fullscreen);

    private:
        static Window* s_Instance;

        int m_width, m_height;
        std::string m_title;
        bool m_fullscreen;
        bool m_isGLFWInitialized;
        bool m_markedToClose;

        GLFWwindow* m_NativeWin;
    };
}
#endif // __WINDOW_H__