#include "vkutils.hpp"

constexpr uint32_t width = 800;
constexpr uint32_t height = 600;

class Application
{
public:
	void run() {
		initWindow();
		initVulkan();

		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}

		glfwDestroyWindow(window);
		glfwTerminate();
	}

private:
	
	GLFWwindow* window = nullptr;

	void initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(width, height, "vulkanRaytracing", nullptr, nullptr);
	}

	void initVulkan() {

	}
};

int main() {
	Application app;
	app.run();
	return 0;
}