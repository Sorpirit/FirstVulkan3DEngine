#include "sorp_v_window.hpp"

#include <stdexcept>

namespace sorp_v
{
	SorpWindow::SorpWindow(int w, int h, std::string name) :
		width{w}, height{h}, windowName{name}
	{
		init();
	}

	SorpWindow::~SorpWindow() 
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void SorpWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface");
		}
	}

	void SorpWindow::init() 
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
	}
}