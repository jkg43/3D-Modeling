#pragma once
#include "StandardIncludes.h"
#include "Camera.h"
#include "RenderObject.h"
#include "MemoryManager.h"
#include "Engine.h"
#include "ModelObject.h"


typedef struct VulkanGlobals
{
	GLFWwindow* window;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	VkQueue graphicsQueue, presentQueue;
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	VkRenderPass renderPass;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkCommandPool commandPool, localCommandPool;
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	bool frameBufferResized = false;
	std::vector<VkBuffer> uniformBuffers;
	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;
	
	
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;


	
	
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	UniformBufferObject ubo;

	//std::vector<Vertex> vertices;
	//std::vector<uint32_t> indices;
	//VkBuffer vertexBuffer;
	//VkDeviceMemory vertexBufferMemory;
	//VkBuffer indexBuffer;
	//VkDeviceMemory indexBufferMemory;

	std::vector<RenderObject*> renderObjects;

	Camera cam = Camera();

	MemoryManager bufferMemManager = MemoryManager(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	bool drawDebug = true, drawDemo = false;

	VkDescriptorPool imguiPool;

	glm::vec4 convertedMousePos;
	
	glm::vec4 mousePosVector;

	glm::vec3 camRay;

	float frameTime = 0;

	std::vector<float> debugValues;

	Engine engine;

	std::vector<VkBuffer> computeShaderStorageBuffers;



	VkPipeline lineGraphicsPipeline;
	VkPipelineLayout linePipelineLayout;

	VkDescriptorSetLayout lineDescriptorSetLayout;
	std::vector<VkDescriptorSet> lineDescriptorSets;

	std::vector<ModelObject> modelObjects;


}VulkanGlobals;

