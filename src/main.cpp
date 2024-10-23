#include "vkutils.hpp"

constexpr uint32_t width = 800;
constexpr uint32_t height = 600;

struct Buffer {
	vk::UniqueBuffer buffer;
	vk::UniqueDeviceMemory memory;
	vk::DeviceAddress address;

	void init(vk::PhysicalDevice physicalDevice,
		vk::Device device,
		vk::DeviceSize size,
		vk::BufferUsageFlags usage,
		vk::MemoryPropertyFlags memoryProperty,
		const void* data = nullptr) {
		// create buffer
		vk::BufferCreateInfo createInfo{};
		createInfo.setSize(size);
		createInfo.setUsage(usage);
		buffer = device.createBufferUnique(createInfo);

		//Allocate memory
		vk::MemoryAllocateFlagsInfo allocateFlags{};
		if (usage & vk::BufferUsageFlagBits::eShaderDeviceAddress) {
			allocateFlags.flags = vk::MemoryAllocateFlagBits::eDeviceAddress;
		}

		vk::MemoryRequirements memoryReq = 
			device.getBufferMemoryRequirements(*buffer);
		uint32_t memoryType = vkutils::getMemoryType(physicalDevice,
			memoryReq, memoryProperty);
		vk::MemoryAllocateInfo allocateInfo{};
		allocateInfo.setAllocationSize(memoryReq.size);
		allocateInfo.setMemoryTypeIndex(memoryType);
		allocateInfo.setPNext(&allocateFlags);
		memory = device.allocateMemoryUnique(allocateInfo);

		device.bindBufferMemory(*buffer, *memory, 0);

		if (data) {
			void* mappedPtr = device.mapMemory(*memory, 0, size);
			memcpy(mappedPtr, data, size);
			device.unmapMemory(*memory);
		}

		if (usage & vk::BufferUsageFlagBits::eShaderDeviceAddress) {
			vk::BufferDeviceAddressInfo addressInfo{};
			addressInfo.setBuffer(*buffer);
			address = device.getBufferAddressKHR(&addressInfo);
		}
	}
};

struct Vertex {
	float pose[3];
};

struct AccelStruct {
	vk::UniqueAccelerationStructureKHR accel;
	Buffer buffer;
};

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

	vk::SurfaceFormatKHR surfaceFormat;
	vk::UniqueSwapchainKHR swapchain;
	std::vector<vk::Image> swapchainImages;
	std::vector<vk::UniqueImageView> swapchainImageViews;

	AccelStruct bottomAccel{};

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
		VkPhysicalDeviceProperties physProp;
		vkGetPhysicalDeviceProperties(physicalDevice, &physProp);
		std::cout << "Device Name: " << physProp.deviceName << std::endl;

		queueFamilyIndex = vkutils::findGeneralQueueFamily(physicalDevice, *surface);
		std::cout << "queue family index: " << queueFamilyIndex << std::endl;
		device = vkutils::createLogicalDevice(physicalDevice, queueFamilyIndex, deviceExtensions);
		queue = device->getQueue(queueFamilyIndex, 0);

		commandPool = vkutils::createCommandPool(*device, queueFamilyIndex);
		commandBuffer = vkutils::createCommandBuffer(*device, *commandPool);

		surfaceFormat = vkutils::chooseSurfaceFormat(physicalDevice, *surface);
		swapchain = vkutils::createSwapchain(
			physicalDevice, *device, *surface, queueFamilyIndex,
			vk::ImageUsageFlagBits::eStorage, surfaceFormat,
			width, height);

		swapchainImages = device->getSwapchainImagesKHR(*swapchain);

		createSwapchainImageViews();

		createBottomLevelAS();
	}

	void createSwapchainImageViews() {
		for (auto image : swapchainImages) {
			vk::ImageViewCreateInfo createInfo{};
			createInfo.setImage(image);
			createInfo.setViewType(vk::ImageViewType::e2D);
			createInfo.setFormat(surfaceFormat.format);
			createInfo.setComponents({ vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA });
			createInfo.setSubresourceRange({ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
			swapchainImageViews.push_back(device->createImageViewUnique(createInfo));
		}

		vkutils::oneTimeSubmit(*device, *commandPool, queue,
			[&](vk::CommandBuffer commandBuffer) {
				for (auto image : swapchainImages) {
					vkutils::setImageLayout(commandBuffer, image,
						vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);
				}
			});
	};

	void createBottomLevelAS() {
		std::cout << "Create BLAS\n";

		std::vector<Vertex> vertices = {
			{{1.0f, 1.0f, 0.0f}},
			{{-1.0f, 1.0f, 0.0f}},
			{{0.0f, -1.0f, 0.0f}},
		};
		std::vector<uint32_t> indices = { 0, 1, 2 };

		vk::BufferUsageFlags bufferUsage{
			vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
			vk::BufferUsageFlagBits::eShaderDeviceAddress };

		vk::MemoryPropertyFlags memoryProperty{
			vk::MemoryPropertyFlagBits::eHostVisible |
			vk::MemoryPropertyFlagBits::eHostCoherent};

		Buffer vertexBuffer;
		Buffer indexBuffer;

		vertexBuffer.init(physicalDevice, *device, 
						  vertices.size() * sizeof(Vertex), bufferUsage, 
						  memoryProperty, vertices.data());

		indexBuffer.init(physicalDevice, *device, 
						 indices.size() * sizeof(uint32_t), bufferUsage, 
						 memoryProperty, indices.data());

		vk::AccelerationStructureGeometryTrianglesDataKHR triangles{};
		triangles.setVertexFormat(vk::Format::eR32G32B32Sfloat);
		triangles.setVertexData(vertexBuffer.address);
		triangles.setVertexStride(sizeof(Vertex));
		triangles.setMaxVertex(static_cast<uint32_t>(vertices.size()));
		triangles.setIndexType(vk::IndexType::eUint32);
		triangles.setIndexData(indexBuffer.address);

		vk::AccelerationStructureGeometryKHR geometry{};
		geometry.setGeometryType(vk::GeometryTypeKHR::eTriangles);
		geometry.setGeometry({ triangles });
		geometry.setFlags(vk::GeometryFlagBitsKHR::eOpaque);
	}
};

int main() {
	Application app;
	app.run();
	return 0;
}