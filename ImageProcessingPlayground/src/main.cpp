#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include "Window.h"
#include "Assert.h"

constexpr uint32_t WIN_WIDTH = 640;
constexpr uint32_t WIN_HEIGHT = 480;
constexpr const char* WIN_TITLE = "OpenCV Playground window";

int main(int argc, char** argv)
{
    playground::Window* win = playground::Window::Create(WIN_WIDTH, WIN_HEIGHT, WIN_TITLE, false);

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
        win->RenderSomething();
    }
    ASSERT(win != 0, "Window is null");
    return 0;
}