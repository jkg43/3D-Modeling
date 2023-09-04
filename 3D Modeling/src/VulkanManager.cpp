#include "Constants.h"
#include <array>
#include <algorithm>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include<tiny_obj_loader.h>
#include <chrono>
#include "RenderObject.h"
#include <iostream>


VulkanGlobals vg;
int currentFrame=0;

void initVulkan()
{
	createVulkanInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();


	vg.bufferMemManager.init();

	createDescriptorSetLayout();
	createSwapchainComponents();
	createCommandPools();
	createTextureImage(TEXTURE_PATH,vg.textureImage,vg.textureImageMemory);
	createTextureImageView();
	createTextureSampler();
	createRenderObjects();
	createShaderObjects();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createCommandBuffers();
	createSyncObjects();
}



void mainLoop()
{



	while (!glfwWindowShouldClose(vg.window))
	{
		glfwPollEvents();
		processInputs();
		updateLogicState();
		createUI();
		drawFrame();
	}

	vkDeviceWaitIdle(vg.device);
}

static void createUI()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (vg.drawDemo)
	{
		ImGui::ShowDemoWindow();
	}
	if (vg.drawDebug)
	{
		showDebugMenu();
	}
}

static void drawFrame()
{
	vkWaitForFences(vg.device, 1, &vg.inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(vg.device, vg.swapChain, UINT64_MAX, vg.imageAvailableSemaphores[currentFrame],
		VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("Failed to acquire swap chain image");
	}

	vkResetFences(vg.device, 1, &vg.inFlightFences[currentFrame]);

	vkResetCommandBuffer(vg.commandBuffers[currentFrame], 0);

	ImGui::Render();

	//start normal render pass
	startRenderPass(vg.commandBuffers[currentFrame], imageIndex);

	//start normal subpass

	int i = 0;
	for (RenderObject& o : vg.renderObjects)
	{
		drawRenderObject(o, vg.commandBuffers[currentFrame],i);
		i++;
	}

	
	if (vg.engine.displayTranslateAxes)
	{
		for (RenderObject &o : vg.engine.translateAxes)
		{
			o.transform.position = vg.engine.axesPos;
			drawRenderObject(o, vg.commandBuffers[currentFrame], i);
			i++;
		}
	}
	

	//start line subpass

	vkCmdNextSubpass(vg.commandBuffers[currentFrame], VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(vg.commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, vg.lineGraphicsPipeline);


	i = 0;
	for (ModelObject& o : vg.modelObjects)
	{
		drawModelObjectLines(o, vg.commandBuffers[currentFrame], i);
		i++;
	}



	//draw gui

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vg.commandBuffers[currentFrame]);



	endRenderPass(vg.commandBuffers[currentFrame]);
	//end render pass




	updateUniformBuffer(currentFrame);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[] = { vg.imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &vg.commandBuffers[currentFrame];
	VkSemaphore signalSemaphores[] = { vg.renderFinishedSemaphores[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	if (vkQueueSubmit(vg.graphicsQueue, 1, &submitInfo, vg.inFlightFences[currentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit draw command buffer");
	}




	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	VkSwapchainKHR swapChains[] = { vg.swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(vg.presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vg.frameBufferResized)
	{
		vg.frameBufferResized = false;
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to present swap chain image");
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

static void drawRenderObject(RenderObject& o, VkCommandBuffer commandBuffer, int i)
{
	if (o.verticesChanged)
	{
		reallocateVertexBuffer(o, i);
		o.verticesChanged = false;
		//printf("Reallocating %d\n", i);
	}

	VkBuffer vertexBuffers[] = { o.vertexBuffer };
	VkDeviceSize offsets[] = { 0 };

	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(commandBuffer, o.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vg.pipelineLayout, 0, 1,
		&vg.descriptorSets[currentFrame], 0, nullptr);


	PushConstantObject renderData;
	renderData.transform = o.transform.getMatrix();

	vkCmdPushConstants(commandBuffer, vg.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, 
		sizeof(PushConstantObject), &renderData);


	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(o.indices.size()), 1, 0, 0, 0);
}

static void drawModelObjectLines(ModelObject& o, VkCommandBuffer commandBuffer, int i)
{

	VkBuffer vertexBuffers[] = { o.ro->vertexBuffer };
	VkDeviceSize offsets[] = { 0 };

	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(commandBuffer, o.lineIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vg.linePipelineLayout, 0, 1,
		&vg.lineDescriptorSets[currentFrame], 0, nullptr);


	PushConstantObject renderData;
	renderData.transform = o.ro->transform.getMatrix();
	renderData.data = glm::vec4(0, 1.0f, 0, 0);

	vkCmdPushConstants(commandBuffer, vg.linePipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
		sizeof(PushConstantObject), &renderData);


	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(o.indices.size()), 1, 0, 0, 0);
}

static void reallocateVertexBuffer(RenderObject& o,int i)
{
	VkDeviceSize bufferSize = sizeof(o.vertices[0]) * o.vertices.size();

	void* data;
	vkMapMemory(vg.device, vg.bufferMemManager.memory, vg.bufferMemManager.getOffset(o.vertexStagingBuffer), bufferSize, 0, &data);
	memcpy(data, o.vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(vg.device, vg.bufferMemManager.memory);

	if (o.numVerticesInBuffer != o.vertices.size())
	{
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			o.vertexBuffer, "Vertex buffer " + std::to_string(i));
	}

	copyBuffer(o.vertexStagingBuffer, o.vertexBuffer, bufferSize);

}

static void startRenderPass(VkCommandBuffer commandBuffer,uint32_t imageIndex)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;
	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to begin recording command buffer");
	}

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { {0.5f,0.5f,0.5f,1.0f} };
	clearValues[1].depthStencil = { 1.0f,0 };

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = vg.renderPass;
	renderPassInfo.framebuffer = vg.swapChainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0,0 };
	renderPassInfo.renderArea.extent = vg.swapChainExtent;
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vg.graphicsPipeline);
}

static void endRenderPass(VkCommandBuffer commandBuffer)
{
	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to record command buffer");
	}
}

static void updateUniformBuffer(uint32_t currentImage)
{
	static auto prevTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();

	float time = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - prevTime).count();

	vg.frameTime = time;

	prevTime = currentTime;
	/*
	////matrix to rotate, angle in radians, axis of rotation
	//vg.ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	////camera pos, target pos, up vector
	//vg.ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	////FOV in y direction(rads), aspect ratio, near clipping plane, far clipping plane
	//vg.ubo.proj = glm::perspective(glm::radians(45.0f), vg.swapChainExtent.width / (float)vg.swapChainExtent.height, 0.1f, 10.0f);
	//vg.ubo.proj[1][1] *= -1; //flip y axis


	//for (size_t i = 0; i < vg.renderObjects.size(); i++)
	//{
	//	vg.ubo.model[i] = vg.renderObjects[i].transform.getModelMatrix();
	//}

	//glm::vec3 target = vg.cam.pos;
	//target.x += sin(vg.cam.direction.x);
	//target.y += sin(vg.cam.direction.y);
	//target.z += sin(vg.cam.direction.z);

	//vg.ubo.view = glm::lookAt(vg.cam.pos, target, glm::vec3(0.0f, 0.0f, 1.0f));
	//vg.ubo.view = glm::mat4(1.0f);
	//glm::rotate(vg.ubo.view, vg.cam.direction.x, glm::vec3(1.0f, 0.0f, 0.0f));
	//glm::rotate(vg.ubo.view, vg.cam.direction.y, glm::vec3(0.0f, 1.0f, 0.0f));
	//glm::rotate(vg.ubo.view, vg.cam.direction.z, glm::vec3(0.0f, 0.0f, 1.0f));

	//glm::mat4 view = vg.cam.transform.getMatrix();
	//glm::mat4 view = glm::lookAtRH(vg.cam.transform.position, vg.cam.targetPos, glm::vec3(0, 0, 1));
	//glm::mat4 view = glm::mat4_cast(-vg.cam.transform.orientation) * glm::translate(-vg.cam.transform.position);
	*/
	glm::mat4 view = vg.cam.arcballViewMatrix();

	glm::mat4 proj = glm::perspective(glm::radians(45.0f), vg.swapChainExtent.width / (float)vg.swapChainExtent.height, 
		NEAR_CLIPPING_DIST, FAR_CLIPPING_DIST);
	proj[1][1] *= -1;

	vg.ubo.projXview = proj * view;

	void* data;
	vkMapMemory(vg.device, vg.bufferMemManager.memory, vg.bufferMemManager.getOffset(vg.uniformBuffers[currentImage]), 
		sizeof(vg.ubo), 0, &data);
	memcpy(data, &vg.ubo, sizeof(vg.ubo));
	vkUnmapMemory(vg.device, vg.bufferMemManager.memory);
}

static void createVulkanInstance()
{
	if (enableValidationLayers && !checkValidationLayerSupport())
	{
		throw std::runtime_error("Validation layers requested but not available");
	}
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "3D Modeling";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = getRequiredExtensions();

	uint32_t numExtensions = static_cast<uint32_t>(extensions.size());

	createInfo.enabledExtensionCount = numExtensions;
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &vg.instance) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Vulkan instance");
	}
}

static std::vector<const char*> getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	if (enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	return extensions;
}

static void createDescriptorSetLayout()
{
	//normal descriptor set layout
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding,samplerLayoutBinding };

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(vg.device, &layoutInfo, nullptr, &vg.descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor set layout");
	}


	//line descriptor set layout
	VkDescriptorSetLayoutCreateInfo lineLayoutInfo{};
	lineLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	lineLayoutInfo.bindingCount = 1;
	lineLayoutInfo.pBindings = &uboLayoutBinding;

	if (vkCreateDescriptorSetLayout(vg.device, &lineLayoutInfo, nullptr, &vg.lineDescriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create line descriptor set layout");
	}
	
}

static void createSwapchainComponents()
{
	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createLineGraphicsPipeline();
	createColorResources();
	createDepthResources();
	createFramebuffers();
}

static void createSwapChain()
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport();

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = vg.surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(vg.physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(),indices.presentFamily.value() };
	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(vg.device, &createInfo, nullptr, &vg.swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swap chain");
	}

	vkGetSwapchainImagesKHR(vg.device, vg.swapChain, &imageCount, nullptr);
	vg.swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(vg.device, vg.swapChain, &imageCount, vg.swapChainImages.data());

	vg.swapChainImageFormat = surfaceFormat.format;
	vg.swapChainExtent = extent;
}

static SwapChainSupportDetails querySwapChainSupport()
{
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vg.physicalDevice, vg.surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(vg.physicalDevice, vg.surface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(vg.physicalDevice, vg.surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(vg.physicalDevice, vg.surface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(vg.physicalDevice, vg.surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}
	return availableFormats[0];
}

static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		int width, height;
		glfwGetFramebufferSize(vg.window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		return actualExtent;
	}
}

static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, vg.surface, &presentSupport);
		if (presentSupport)
		{
			indices.presentFamily = i;
		}
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}
		if (indices.isComplete())
		{
			break;
		}

		i++;
	}

	return indices;
}

static void createImageViews()
{
	vg.swapChainImageViews.resize(vg.swapChainImages.size());
	for (size_t i = 0; i < vg.swapChainImages.size(); i++)
	{
		vg.swapChainImageViews[i] = createImageView(vg.swapChainImages[i], vg.swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}


static void createRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = vg.swapChainImageFormat;
	colorAttachment.samples = vg.msaaSamples;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = findDepthFormat();
	depthAttachment.samples = vg.msaaSamples;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachmentResolve{};
	colorAttachmentResolve.format = vg.swapChainImageFormat;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;



	VkAttachmentReference colorAttachmentReference{};
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentReference{};
	depthAttachmentReference.attachment = 1;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentResolveReference{};
	colorAttachmentResolveReference.attachment = 2;
	colorAttachmentResolveReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;



	//VkAttachmentDescription lineColorAttachment{};
	//colorAttachment.format = vg.swapChainImageFormat;
	//colorAttachment.samples = vg.msaaSamples;
	//colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	//colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	//colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	//colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	//colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	//colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//VkAttachmentDescription lineDepthAttachment{};
	//depthAttachment.format = findDepthFormat();
	//depthAttachment.samples = vg.msaaSamples;
	//depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	//depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	//depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	//depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	//depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	//depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//VkAttachmentReference lineColorAttachmentReference{};
	//colorAttachmentReference.attachment = 3;
	//colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//VkAttachmentReference lineDepthAttachmentReference{};
	//depthAttachmentReference.attachment = 4;
	//depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;



	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentReference;
	subpass.pDepthStencilAttachment = &depthAttachmentReference;
	subpass.pResolveAttachments = &colorAttachmentResolveReference;
	

	VkSubpassDescription lineSubpass{};
	lineSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	lineSubpass.colorAttachmentCount = 1;
	lineSubpass.pColorAttachments = &colorAttachmentReference;
	lineSubpass.pDepthStencilAttachment = &depthAttachmentReference;
	lineSubpass.pResolveAttachments = &colorAttachmentResolveReference;



	

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkSubpassDependency lineDependency{};
	lineDependency.srcSubpass = 0;
	lineDependency.dstSubpass = 1;
	lineDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	lineDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	lineDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	lineDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;



	std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
	std::array<VkSubpassDescription, 2> subpasses = { subpass, lineSubpass };
	std::array<VkSubpassDependency, 2> dependencies = { dependency, lineDependency };
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
	renderPassInfo.pSubpasses = subpasses.data();
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();
	if (vkCreateRenderPass(vg.device, &renderPassInfo, nullptr, &vg.renderPass) != VK_SUCCESS)
	{
		throw new std::runtime_error("Failed to create render pass");
	}
}


static VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(vg.physicalDevice, format, &props);
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}
	throw std::runtime_error("Failed to find supported format");
}

static VkFormat findDepthFormat()
{
	return findSupportedFormat({ VK_FORMAT_D32_SFLOAT,VK_FORMAT_D32_SFLOAT_S8_UINT,VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

static void createGraphicsPipeline()
{
	auto vertShaderCode = readFile("shaders/vert.spv");
	auto fragShaderCode = readFile("shaders/frag.spv");

	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo,fragShaderStageInfo };

	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)vg.swapChainExtent.width;
	viewport.height = (float)vg.swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0,0 };
	scissor.extent = vg.swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = vg.msaaSamples;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &vg.descriptorSetLayout;

	//push constant stuff
	VkPushConstantRange pushConstant;
	pushConstant.offset = 0;
	pushConstant.size = sizeof(PushConstantObject);
	pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	pipelineLayoutInfo.pPushConstantRanges = &pushConstant;
	pipelineLayoutInfo.pushConstantRangeCount = 1;


	if (vkCreatePipelineLayout(vg.device, &pipelineLayoutInfo, nullptr, &vg.pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout");
	}



	VkStencilOpState stencilOp{};
	stencilOp.compareOp = VK_COMPARE_OP_ALWAYS;
	stencilOp.writeMask = 0xFFFFFFFF;
	stencilOp.compareMask = 0xFFFFFFFF;
	stencilOp.failOp = VK_STENCIL_OP_KEEP;
	stencilOp.depthFailOp = VK_STENCIL_OP_KEEP;
	stencilOp.passOp = VK_STENCIL_OP_REPLACE;
	stencilOp.reference = 0;

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f;
	depthStencil.maxDepthBounds = 1.0f;
	depthStencil.stencilTestEnable = VK_TRUE;
	depthStencil.front = stencilOp;
	depthStencil.back = stencilOp;

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.layout = vg.pipelineLayout;
	pipelineInfo.renderPass = vg.renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.pDepthStencilState = &depthStencil;

	if (vkCreateGraphicsPipelines(vg.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vg.graphicsPipeline) != VK_SUCCESS)
	{
		throw new std::runtime_error("Failed to create graphics pipeline");
	}


	vkDestroyShaderModule(vg.device, fragShaderModule, nullptr);
	vkDestroyShaderModule(vg.device, vertShaderModule, nullptr);

}

static void createLineGraphicsPipeline()
{
	auto vertShaderCode = readFile("shaders/lineVert.spv");
	auto fragShaderCode = readFile("shaders/lineFrag.spv");

	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo,fragShaderStageInfo };

	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
	inputAssembly.primitiveRestartEnable = VK_TRUE;
	

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)vg.swapChainExtent.width;
	viewport.height = (float)vg.swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0,0 };
	scissor.extent = vg.swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
	rasterizer.lineWidth = 2.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = vg.msaaSamples;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &vg.lineDescriptorSetLayout;

	//push constant stuff
	VkPushConstantRange pushConstant;
	pushConstant.offset = 0;
	pushConstant.size = sizeof(PushConstantObject);
	pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	pipelineLayoutInfo.pPushConstantRanges = &pushConstant;
	pipelineLayoutInfo.pushConstantRangeCount = 1;


	if (vkCreatePipelineLayout(vg.device, &pipelineLayoutInfo, nullptr, &vg.linePipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout");
	}


	VkStencilOpState stencilOp{};
	stencilOp.compareOp = VK_COMPARE_OP_EQUAL;
	stencilOp.failOp = VK_STENCIL_OP_KEEP;
	stencilOp.depthFailOp = VK_STENCIL_OP_KEEP;
	stencilOp.passOp = VK_STENCIL_OP_KEEP;
	stencilOp.compareMask = 0xFFFFFFFF;
	stencilOp.writeMask = 0xFFFFFFFF;
	stencilOp.reference = 1;


	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f;
	depthStencil.maxDepthBounds = 1.0f;
	depthStencil.stencilTestEnable = VK_TRUE;
	depthStencil.front = stencilOp;
	depthStencil.back = stencilOp;


	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.layout = vg.linePipelineLayout;
	pipelineInfo.renderPass = vg.renderPass;
	pipelineInfo.subpass = 1;
	pipelineInfo.pDepthStencilState = &depthStencil;

	if (vkCreateGraphicsPipelines(vg.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vg.lineGraphicsPipeline) != VK_SUCCESS)
	{
		throw new std::runtime_error("Failed to create line graphics pipeline");
	}


	vkDestroyShaderModule(vg.device, fragShaderModule, nullptr);
	vkDestroyShaderModule(vg.device, vertShaderModule, nullptr);

}

static VkShaderModule createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(vg.device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create shader module");
	}
	return shaderModule;
}

static void createColorResources()
{
	VkFormat colorFormat = vg.swapChainImageFormat;
	createImage(vg.swapChainExtent.width, vg.swapChainExtent.height, vg.msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vg.colorImage, vg.colorImageMemory);
	vg.colorImageView = createImageView(vg.colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
}

static void createDepthResources()
{
	VkFormat depthFormat = findDepthFormat();
	
	createImage(vg.swapChainExtent.width, vg.swapChainExtent.height, vg.msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vg.depthImage, vg.depthImageMemory);
	vg.depthImageView = createImageView(vg.depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

}

static void createFramebuffers()
{
	vg.swapChainFramebuffers.resize(vg.swapChainImageViews.size());
	for (size_t i = 0; i < vg.swapChainImageViews.size(); i++)
	{
		std::array<VkImageView, 3> attachments = { vg.colorImageView, vg.depthImageView,
			vg.swapChainImageViews[i]};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = vg.renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = vg.swapChainExtent.width;
		framebufferInfo.height = vg.swapChainExtent.height;
		framebufferInfo.layers = 1;
		if (vkCreateFramebuffer(vg.device, &framebufferInfo, nullptr, &vg.swapChainFramebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create frame buffer");
		}
	}
}

static void recreateSwapChain()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(vg.window, &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(vg.window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(vg.device);

	cleanupSwapChain();

	createSwapchainComponents();
}

static void createCommandPools()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(vg.physicalDevice);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	if (vkCreateCommandPool(vg.device, &poolInfo, nullptr, &vg.commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create command pool");
	}

	VkCommandPoolCreateInfo localPoolInfo{};
	localPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	localPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	localPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	if (vkCreateCommandPool(vg.device, &localPoolInfo, nullptr, &vg.localCommandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create local command pool");
	}
}

static void createTextureImage(std::string path,VkImage& textureImage,VkDeviceMemory& textureImageMemory)
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;
	if (!pixels)
	{
		throw std::runtime_error("Failed to load texture image");
	}

	VkBuffer stagingBuffer;
	createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,"Texture image staging");
	void* data;
	vkMapMemory(vg.device, vg.bufferMemManager.memory, vg.bufferMemManager.getOffset(stagingBuffer), imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(vg.device, vg.bufferMemManager.memory);
	stbi_image_free(pixels);

	createImage(texWidth, texHeight, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT |
		VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vg.bufferMemManager.deallocate(stagingBuffer);
	vkDestroyBuffer(vg.device, stagingBuffer, nullptr);
	//vkFreeMemory(vg.device, stagingBufferMemory, nullptr);
}

static void createTextureImageView()
{
	vg.textureImageView = createImageView(vg.textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

static void createTextureSampler()
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(vg.physicalDevice, &properties);
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	if (vkCreateSampler(vg.device, &samplerInfo, nullptr, &vg.textureSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create texture sampler");
	}
}

static void createRenderObjects()
{

	vg.renderObjects.resize(11);

	vg.modelObjects.resize(3);

	loadModelObjectCube(vg.modelObjects[0],&vg.renderObjects[8]);
	vg.modelObjects[0].ro->transform.position.y = -3.0f;

	loadModelObjectCylinder(vg.modelObjects[2], &vg.renderObjects[10], 1, 2, 50);
	vg.modelObjects[2].ro->transform.position.x = 3.0f;


	loadModel(vg.renderObjects[0],MODEL_PATH);

	//loadCube(vg.renderObjects[1]);

	float axisWidth = 0.075f;

	loadCustomCube(vg.renderObjects[1], axisWidth, axisWidth, 1.0f, 0, 0, 1.0f);
	loadCustomCube(vg.renderObjects[2], axisWidth, 1.0f, axisWidth, 0, 1.0f, 0);
	loadCustomCube(vg.renderObjects[3], 1.0f, axisWidth, axisWidth, 1.0f, 0, 0);
	loadCustomCube(vg.renderObjects[4], 0.1f, 0.1f, 0.1f, 1.0f, 0.6f, 0.2f);
	loadCustomCube(vg.renderObjects[5], 0.1f, 0.1f, 0.1f, 1.0f, 0, 0);
	loadCustomCube(vg.renderObjects[6], 1.0f, 1.0f, 1.0f, 0, 1.0f, 0);
	vg.renderObjects[6].transform.position.y = 3.0f;


	vg.renderObjects[7].vertices = {
		{{0,0,1.0f},{1.0f,1.0f,1.0f}},
		{{0,1.0f,1.0f},{1.0f,1.0f,1.0f}}
	};
	vg.renderObjects[7].indices = { 0,1,0 };


	vg.renderObjects[9].vertices =
	{
		{{0.0f,0.0f,0.0f},{0.0f,0.0f,1.0f}},
		{{0.0f,0.0f,2.0f},{0.0f,0.0f,1.0f}},
		{{0.0f,2.0f,0.0f},{0.0f,0.0f,1.0f}},
		{{0.0f,2.0f,2.0f},{0.0f,0.0f,1.0f}},
		{{0.0f,0.0f,4.0f},{0.0f,0.0f,1.0f}},
		{{0.0f,2.0f,4.0f},{0.0f,0.0f,1.0f}},
		{{2.0f,0.0f,0.0f},{0.0f,0.0f,1.0f}}
	};

	vg.renderObjects[9].indices = { 
		0,1,3,
		0,2,3,
		1,4,5,
		0,2,6
	};

	int length = vg.renderObjects[9].indices.size() / 3;

	vg.renderObjects[9].transform.position.z = -5.0f;

	vg.modelObjects[1].ro = &vg.renderObjects[9];

	vg.modelObjects[1].planes.resize(length);

	for (int k = 0; k < length; k++)
	{
		vg.modelObjects[1].planes[k].edges.resize(3);
		vg.modelObjects[1].planes[k].vertices.resize(3);
		for (int j = 0; j < 3; j++)
		{
			vg.modelObjects[1].planes[k].vertices[j] = 
				&vg.renderObjects[9].vertices[vg.renderObjects[9].indices[k * 3 + j]];
			vg.modelObjects[1].planes[k].edges[j] = Edge(
				&vg.renderObjects[9].vertices[vg.renderObjects[9].indices[k * 3 + j]],
				&vg.renderObjects[9].vertices[vg.renderObjects[9].indices[k * 3 + (j + 1) % 3]]);
		}
	}
	vg.modelObjects[1].indices = vg.renderObjects[9].indices;


	int i = 0;
	for (RenderObject& o : vg.renderObjects)
	{
		createVertexBuffer(o,i);
		createIndexBuffer(o,i);
		i++;
	}

	i = 0;
	for (ModelObject& o : vg.modelObjects)
	{
		createModelIndexBuffer(o, i);
		i++;
	}




}

static void loadModel(RenderObject& o,std::string path)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
	{
		throw std::runtime_error(warn + err);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			Vertex vertex{};
			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};
			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};
			vertex.color = { 1.0f,1.0f,1.0f };

			if (uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(o.vertices.size());
				o.vertices.push_back(vertex);
			}
			o.indices.push_back(uniqueVertices[vertex]);
		}
	}
}


static void createShaderObjects()
{
	vg.computeShaderStorageBuffers.resize(MAX_FRAMES_IN_FLIGHT);
}



static void createUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	vg.uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vg.uniformBuffers[i], "Uniform buffer " + std::to_string(i));
	}
}

static void createDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 2> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * 2;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);


	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * 3;

	if (vkCreateDescriptorPool(vg.device, &poolInfo, nullptr, &vg.descriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor pool");
	}
}

static void createDescriptorSets()
{
	//normal descriptor sets
	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, vg.descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = vg.descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	vg.descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(vg.device, &allocInfo, vg.descriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate descriptor sets");
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = vg.uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = vg.textureImageView;
		imageInfo.sampler = vg.textureSampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = vg.descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;
		descriptorWrites[0].pImageInfo = nullptr;
		descriptorWrites[0].pTexelBufferView = nullptr;
		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = vg.descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;
		vkUpdateDescriptorSets(vg.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}

	//line descriptor sets
	std::vector<VkDescriptorSetLayout> lineLayouts(MAX_FRAMES_IN_FLIGHT, vg.lineDescriptorSetLayout);
	VkDescriptorSetAllocateInfo lineAllocInfo{};
	lineAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	lineAllocInfo.descriptorPool = vg.descriptorPool;
	lineAllocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	lineAllocInfo.pSetLayouts = lineLayouts.data();

	vg.lineDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(vg.device, &lineAllocInfo, vg.lineDescriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate line descriptor sets");
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = vg.uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = vg.lineDescriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;
		descriptorWrites[0].pImageInfo = nullptr;
		descriptorWrites[0].pTexelBufferView = nullptr;
		vkUpdateDescriptorSets(vg.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

static void createCommandBuffers()
{
	vg.commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = vg.commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)vg.commandBuffers.size();
	if (vkAllocateCommandBuffers(vg.device, &allocInfo, vg.commandBuffers.data()) != VK_SUCCESS)
	{
		throw new std::runtime_error("Failed to allocate command buffers");
	}
}

static void createSyncObjects()
{
	vg.imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	vg.renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	vg.inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(vg.device, &semaphoreInfo, nullptr, &vg.imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(vg.device, &semaphoreInfo, nullptr, &vg.renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(vg.device, &fenceInfo, nullptr, &vg.inFlightFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create sync objects for a frame");
		}
	}
}



void cleanup()
{
	cleanupSwapChain();

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vg.bufferMemManager.deallocate(vg.uniformBuffers[i]);
		vkDestroyBuffer(vg.device, vg.uniformBuffers[i], nullptr);
	}

	vkDestroyDescriptorSetLayout(vg.device, vg.descriptorSetLayout, nullptr);

	vkDestroyDescriptorSetLayout(vg.device, vg.lineDescriptorSetLayout, nullptr);

	vkDestroyDescriptorPool(vg.device, vg.descriptorPool, nullptr);

	vkDestroyDescriptorPool(vg.device, vg.imguiPool, nullptr);
	ImGui_ImplVulkan_Shutdown();

	vkDestroySampler(vg.device, vg.textureSampler, nullptr);
	vkDestroyImageView(vg.device, vg.textureImageView, nullptr);
	vkDestroyImage(vg.device, vg.textureImage, nullptr);
	vkFreeMemory(vg.device, vg.textureImageMemory, nullptr);

	for (RenderObject& o : vg.renderObjects)
	{
		vg.bufferMemManager.deallocate(o.indexBuffer);
		vkDestroyBuffer(vg.device, o.indexBuffer, nullptr);

		vg.bufferMemManager.deallocate(o.vertexBuffer);
		vkDestroyBuffer(vg.device, o.vertexBuffer, nullptr);

		vg.bufferMemManager.deallocate(o.vertexStagingBuffer);
		vkDestroyBuffer(vg.device, o.vertexStagingBuffer, nullptr);
	}

	for (RenderObject &o : vg.engine.translateAxes)
	{
		vg.bufferMemManager.deallocate(o.indexBuffer);
		vkDestroyBuffer(vg.device, o.indexBuffer, nullptr);

		vg.bufferMemManager.deallocate(o.vertexBuffer);
		vkDestroyBuffer(vg.device, o.vertexBuffer, nullptr);

		vg.bufferMemManager.deallocate(o.vertexStagingBuffer);
		vkDestroyBuffer(vg.device, o.vertexStagingBuffer, nullptr);
	}

	for (ModelObject& o : vg.modelObjects)
	{
		vg.bufferMemManager.deallocate(o.lineIndexBuffer);
		vkDestroyBuffer(vg.device, o.lineIndexBuffer, nullptr);
	}


	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(vg.device, vg.imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(vg.device, vg.renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(vg.device, vg.inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(vg.device, vg.localCommandPool, nullptr);
	vkDestroyCommandPool(vg.device, vg.commandPool, nullptr);

	vg.bufferMemManager.cleanup();

	vkDestroyDevice(vg.device, nullptr);
	if (enableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(vg.instance, vg.debugMessenger, nullptr);
	}
	vkDestroySurfaceKHR(vg.instance, vg.surface, nullptr);
	vkDestroyInstance(vg.instance, nullptr);
	glfwDestroyWindow(vg.window);
	glfwTerminate();
}

static void cleanupSwapChain()
{
	vkDestroyImageView(vg.device, vg.colorImageView, nullptr);
	vkDestroyImage(vg.device, vg.colorImage, nullptr);
	vkFreeMemory(vg.device, vg.colorImageMemory, nullptr);

	vkDestroyImageView(vg.device, vg.depthImageView, nullptr);
	vkDestroyImage(vg.device, vg.depthImage, nullptr);
	vkFreeMemory(vg.device, vg.depthImageMemory, nullptr);



	for (auto framebuffer : vg.swapChainFramebuffers)
	{
		vkDestroyFramebuffer(vg.device, framebuffer, nullptr);
	}

	
	vkDestroyPipeline(vg.device, vg.graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(vg.device, vg.pipelineLayout, nullptr);
	
	vkDestroyPipeline(vg.device, vg.lineGraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(vg.device, vg.linePipelineLayout, nullptr);


	vkDestroyRenderPass(vg.device, vg.renderPass, nullptr);


	for (auto imageView : vg.swapChainImageViews)
	{
		vkDestroyImageView(vg.device, imageView, nullptr);
	}
	vkDestroySwapchainKHR(vg.device, vg.swapChain, nullptr);


}