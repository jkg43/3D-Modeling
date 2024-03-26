#include "Constants.h"
#include <iostream>
#include <list>


MemoryManager::MemoryManager(VkMemoryPropertyFlags properties)
{
	this->properties = properties;
	memory = nullptr;
	memoryLeft = DEFAULT_HEAP_SIZE;
}

void MemoryManager::init()
{
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(vg.physicalDevice, &memoryProperties);



	uint32_t desiredMemoryTypeIndex = UINT32_MAX;
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
		if ((memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
			(memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
			desiredMemoryTypeIndex = i;
			printf("Valid Memory Index: %d\n", i);
			break;
		}
	}

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = DEFAULT_HEAP_SIZE;
	allocInfo.memoryTypeIndex = desiredMemoryTypeIndex;

	if (vkAllocateMemory(vg.device, &allocInfo, nullptr, &memory) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate buffer memory");
	}
}

void MemoryManager::cleanup()
{
	vkFreeMemory(vg.device, memory, nullptr);
}

//void MemoryManager::combineBlocks()
//{
//	//will store adjacent unused sections as pairs of offset and size,
//	// so each 2 elements will be one section
//	std::vector<VkDeviceSize> unusedSections;
//	int numUnusedSections = 0;
//	VkDeviceSize currentEndPos = 0;
//	unusedSections.reserve(blocks.size());
//	int i = 0;
//	for (MemBlock& b : blocks)
//	{
//		if (!b.used)
//		{
//			if (b.offset + b.size != currentEndPos)
//			{
//				i++;
//				currentEndPos = b.offset + b.size;
//				unusedSections.data()[i*2] = b.offset;
//				unusedSections.data()[i*2+1] = b.size;
//			}
//			else
//			{
//				currentEndPos = currentEndPos + b.size;
//				unusedSections.data()[i * 2 + 1] += b.size;
//			}
//
//		}
//	}
//	unusedSections.shrink_to_fit();
//
//	for (VkDeviceSize s : unusedSections)
//	{
//		std::cout << s << std::endl;
//	}
//}

void MemoryManager::combineBlocks()
{
	bool prevUnused = false;
	MemBlock* prevBlock = nullptr;

	std::list<MemBlock>::iterator i = blocks.begin();

	while (i != blocks.end())
	{
		if (!(i->used))
		{
			if (prevUnused)
			{
				//std::cout << "Combining - 1st Name: " << prevBlock->name << ", 2nd Name: " << i->name << ", 1st Size: " <<
				//	prevBlock->size << ", 2nd size: " << i->size << ", 1st Offset: " << prevBlock->offset <<
				//	", 2nd Offset: " << i->offset << std::endl;
				prevBlock->size += i->size;
				i = blocks.erase(i);
			}
			else
			{
				prevUnused = true;
				prevBlock = &(*i);
				++i;
			}
		}
		else
		{
			prevUnused = false;
			prevBlock = nullptr;
			++i;
		}
	}
}

int emptyIndex = 0;

AllocResult MemoryManager::allocate(VkDeviceSize size, void *obj, std::string n, VkDeviceSize a)
{
	size = align(size, a);

	size_t i = 0;
	for (MemBlock& b : blocks)
	{
		if (!b.used && b.size >= size + 2 * a /*&& size >= b.size * 0.8*/)
		{
			//std::cout << "Reusing - Old Name: " << b.name << ", New name: " << n << ", Offset: " << b.offset << ", Old Size: " << b.size
			//	<< ", New size: " << size << std::endl;

			b.name = n;

			VkDeviceSize newOffset = align(b.offset, a);

			VkDeviceSize offsetGap = newOffset - b.offset;

			if (newOffset != b.offset)
			{
				std::list<MemBlock>::iterator it = blocks.begin();
				std::advance(it, i);
				blocks.insert(it, MemBlock(b.offset, offsetGap, nullptr, "Empty "
					+ std::to_string(emptyIndex)));
				emptyIndex++;
			}

			b.offset = newOffset;

			VkDeviceSize newSize = b.size - size - offsetGap;

			b.size = size;
			b.object = obj;
			b.used = true;
			
			if (newSize > 0)
			{

				std::list<MemBlock>::iterator it = blocks.begin();
				std::advance(it, i+1);
				blocks.insert(it, MemBlock(b.offset + size, newSize, nullptr, "Empty "
					+ std::to_string(emptyIndex)));
				emptyIndex++;
			}
			
			combineBlocks();

			return ALLOC_SUCCESS;
		}
		i++;
	}
	VkDeviceSize endPos = align(DEFAULT_HEAP_SIZE - memoryLeft, a);
	
	if (size <= memoryLeft)
	{
		blocks.push_back(MemBlock(endPos, size, obj,n));
		memoryLeft -= size;

		combineBlocks();

		return ALLOC_SUCCESS;
	}
	
	return ALLOC_FAIL;
}

MemBlock* MemoryManager::getBlock(void* obj)
{
	for (MemBlock& b : blocks)
	{
		if (b.object == obj)
		{
			return &b;
		}
	}
	return nullptr;
}

VkDeviceSize MemoryManager::getOffset(void* obj)
{
	for (MemBlock& b : blocks)
	{
		if (b.object == obj)
		{
			return b.offset;
		}
	}
	return 0;
}

DeallocResult MemoryManager::deallocate(void* object)
{
	for (MemBlock& b : blocks)
	{
		if (b.object == object)
		{
			b.used = false;
			b.object = nullptr;
			//std::cout << "Dealloc - Name: " << b.name << ", Offset: " << b.offset << ", Size: " << b.size << std::boolalpha
			//	<< ", Used: " << b.used << std::noboolalpha << std::endl;
			break;
		}
	}


	return DEALLOC_NOT_PRESENT;
}


MemBlock::MemBlock(VkDeviceSize o, VkDeviceSize s, void* obj,std::string n)
{
	offset = o;
	size = s;
	object = obj;
	used = obj; //true if obj not null
	name = n;
	//std::cout << "Creating - Name: " << name << ", Offset: " << offset << ", Size: " << size << std::boolalpha <<
	//	", Used: " << used << std::noboolalpha << std::endl;
}