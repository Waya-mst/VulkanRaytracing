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

	vk::UniqueInstance instance;
	vk::UniqueDebugUtilsMessengerEXT debugMessenger;
	vk::UniqueSurfaceKHR surface;

	vk::PhysicalDevice physicalDevice;
	vk::UniqueDevice device;

	vk::Queue queue;
	uint32_t queueFamilyIndex{};

	vk::UniqueCommandPool commandPool;
	vk::UniqueCommandBuffer commandBuffer;

	void initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(width, height, "vulkanRaytracing", nullptr, nullptr);
	}

	void initVulkan() {
		std::vector<const char*> layers = {
			"VK_LAYER_KHRONOS_validation",
		};

		instance = vkutils::createInstance(VK_API_VERSION_1_2, layers);
		debugMessenger = vkutils::createDebugMessenger(*instance);
		surface = vkutils::createSurface(*instance, window);

		std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
			VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
			VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
			VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
			VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
		};
		physicalDevice = vkutils::pickPhysicalDevice(*instance, *surface, deviceExtensions);

		queueFamilyIndex = vkutils::findGeneralQueueFamily(physicalDevice, *surface);
		std::cout << "queue family index" << queueFamilyIndex << std::endl;
		device = vkutils::createLogicalDevice(physicalDevice, queueFamilyIndex, deviceExtensions);
		queue = device->getQueue(queueFamilyIndex, 0);

		commandPool = vkutils::createCommandPool(*device, queueFamilyIndex);
		commandBuffer = vkutils::createCommandBuffer(*device, *commandPool);
	}
};

int main() {
	Application app;
	app.run();
	return 0;
}