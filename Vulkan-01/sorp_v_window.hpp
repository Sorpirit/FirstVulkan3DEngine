#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace sorp_v 
{
	class SorpWindow
	{
	public:
		SorpWindow(int w, int h, std::string name);
		~SorpWindow();

		SorpWindow(const SorpWindow&) = delete;
		SorpWindow &operator=(const SorpWindow&) = delete;

		bool shouldClose() { return glfwWindowShouldClose(window); }
		VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	private:
		GLFWwindow* window;

		std::string windowName;
		const int width;
		const int height;

		void init();
	};
}