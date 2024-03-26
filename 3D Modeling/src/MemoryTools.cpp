#include "Constants.h"
#include <fstream>
#include <iostream>

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(vg.physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}
	throw std::runtime_error("Failed to find suitable memory type");
}

std::vector<char> readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw(std::runtime_error("Failed to open file"));
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	return buffer;
}

void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0,0,0 };
	region.imageExtent = { width,height,1 };

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	endSingleTimeCommands(commandBuffer);
}

void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
	VkBuffer& buffer,std::string name)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(vg.device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create buffer");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(vg.device, buffer, &memRequirements);

	if (memRequirements.alignment == 0x100) {
		printf("");
	}
	
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	//if (vkAllocateMemory(vg.device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	//{
	//	throw std::runtime_error("Failed to allocate buffer memory");
	//}

	if (vg.bufferMemManager.allocate(memRequirements.size, buffer, name,memRequirements.alignment) != ALLOC_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate buffer memory");
	}

	vkBindBufferMemory(vg.device, buffer, vg.bufferMemManager.memory, vg.bufferMemManager.getOffset(buffer));
}

void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(commandBuffer);
}

void createVertexBuffer(RenderObject *o, int i)
{
	VkDeviceSize bufferSize = sizeof(o->vertices[0]) * o->vertices.size();

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		o->vertexStagingBuffer, "Vertex buffer staging " + std::to_string(i));

	void *data;
	vkMapMemory(vg.device, vg.bufferMemManager.memory, vg.bufferMemManager.getOffset(o->vertexStagingBuffer), bufferSize, 0, &data);
	memcpy(data, o->vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(vg.device, vg.bufferMemManager.memory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		o->vertexBuffer, "Vertex buffer " + std::to_string(i));

	copyBuffer(o->vertexStagingBuffer, o->vertexBuffer, bufferSize);

	//Dont deallocate staging buffer, will be resused whenever vertices change

	o->numVerticesInBuffer = o->vertices.size();
}

void createIndexBuffer(RenderObject *o, int i)
{
	VkDeviceSize numIndices = o->indices.size();
	VkDeviceSize bufferSize = numIndices * sizeof(o->indices[0]);

	VkBuffer stagingBuffer;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, "Index buffer staging " + std::to_string(i));

	void *data;
	vkMapMemory(vg.device, vg.bufferMemManager.memory, vg.bufferMemManager.getOffset(stagingBuffer), bufferSize, 0, &data);
	memcpy(data, o->indices.data(), (size_t)bufferSize);
	vkUnmapMemory(vg.device, vg.bufferMemManager.memory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, o->indexBuffer, "Index buffer " + std::to_string(i));

	copyBuffer(stagingBuffer, o->indexBuffer, bufferSize);

	vg.bufferMemManager.deallocate(stagingBuffer);
	vkDestroyBuffer(vg.device, stagingBuffer, nullptr);
	//vkFreeMemory(vg.device, stagingBufferMemory, nullptr);
}

void createModelIndexBuffer(ModelObject &o, int i)
{
	VkDeviceSize numIndices = o.indices.size();
	VkDeviceSize bufferSize = numIndices * sizeof(o.indices[0]);

	VkBuffer stagingBuffer;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, "Line index buffer staging " + std::to_string(i));

	void *data;
	vkMapMemory(vg.device, vg.bufferMemManager.memory, vg.bufferMemManager.getOffset(stagingBuffer), bufferSize, 0, &data);
	memcpy(data, o.indices.data(), (size_t)bufferSize);
	vkUnmapMemory(vg.device, vg.bufferMemManager.memory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, o.lineIndexBuffer, "Index buffer " + std::to_string(i));

	copyBuffer(stagingBuffer, o.lineIndexBuffer, bufferSize);

	vg.bufferMemManager.deallocate(stagingBuffer);
	vkDestroyBuffer(vg.device, stagingBuffer, nullptr);
}

VkDeviceSize align(VkDeviceSize val, VkDeviceSize alignment)
{
	if (alignment <= 0x10)
	{
		return align16(val);
	}
	else
	{
		return align256(val);
	}
}

VkDeviceSize align16(VkDeviceSize val)
{
	if (val % 0x10 == 0)
	{
		return val;
	}
	return ((val >> 4) + 1) << 4;
}

VkDeviceSize align256(VkDeviceSize val)
{
	if (val % 0x100 == 0)
	{
		return val;
	}
	return ((val >> 8) + 1) << 8;
}
