#pragma once
#include "StandardIncludes.h"
#include <string>


typedef enum AllocResult
{
	ALLOC_SUCCESS,
	ALLOC_FAIL
}AllocResult;

typedef enum DeallocResult
{
	DEALLOC_SUCCESS,
	DEALLOC_FAIL,
	DEALLOC_NOT_PRESENT
}DeallocResult;

class MemBlock
{
public:
	VkDeviceSize offset,size;
	void* object;
	bool used;
	std::string name;
	MemBlock(VkDeviceSize o, VkDeviceSize s, void* obj,std::string n);
};

class MemoryManager
{
public:
	MemoryManager(VkMemoryPropertyFlags properties);
	void init();
	void cleanup();
	void combineBlocks();
	AllocResult allocate(VkDeviceSize size, void *obj, std::string n, VkDeviceSize a);
	DeallocResult deallocate(void* object);
	MemBlock* getBlock(void* obj);
	VkDeviceSize getOffset(void* object);
	VkDeviceSize memoryLeft;
	VkDeviceMemory memory;
	VkMemoryPropertyFlags properties;
	std::list<MemBlock> blocks;
	uint32_t alignment = 0x10;
};
