#pragma once
#include "StandardIncludes.h"
#include "RenderObject.h"
void initVulkan();
void mainLoop();
static void drawFrame();
static void createUI();
//static void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
static void updateUniformBuffer(uint32_t currentImage);
static void createVulkanInstance();
static std::vector<const char*> getRequiredExtensions();
static void createDescriptorSetLayout();
static void createSwapchainComponents();
static void createSwapChain();
static SwapChainSupportDetails querySwapChainSupport();
static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice);
static void createImageViews();
static void createRenderPass();
static VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
static VkFormat findDepthFormat();
static void createGraphicsPipeline();
static VkShaderModule createShaderModule(const std::vector<char>& code);
static void createColorResources();
static void createDepthResources();
static void createFramebuffers();
static void recreateSwapChain();
static void createCommandPools();
static void createTextureImage(std::string path, VkImage& textureImage, VkDeviceMemory& textureImageMemory);
static void createTextureImageView();
static void createTextureSampler();
static void loadModel(RenderObject* o,std::string path);
static void createUniformBuffers();
static void createDescriptorPool();
static void createDescriptorSets();
static void createCommandBuffers();
static void createSyncObjects();
static void drawRenderObject(RenderObject& o, VkCommandBuffer commandBuffer,int i);
static void endRenderPass(VkCommandBuffer commandBuffer);
static void startRenderPass(VkCommandBuffer commandBuffer, uint32_t imageIndex);
static void createRenderObjects();
static void reallocateVertexBuffer(RenderObject& o, int i);
static void createShaderObjects();
static void createLineGraphicsPipeline();
static void drawModelObjectLines(ModelObject& o, VkCommandBuffer commandBuffer, int i);


void cleanup();
static void cleanupSwapChain();