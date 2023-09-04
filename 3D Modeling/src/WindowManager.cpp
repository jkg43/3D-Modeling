#include "Constants.h"

void initWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	vg.window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Test", nullptr, nullptr);
	glfwSetFramebufferSizeCallback(vg.window, framebufferResizeCallback);
	initInputCallbacks();

}

static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	vg.frameBufferResized = true;
}

void createSurface()
{
	if (glfwCreateWindowSurface(vg.instance, vg.window, nullptr, &vg.surface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface");
	}
}