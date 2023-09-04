#pragma once
#include "StandardIncludes.h"

void pickPhysicalDevice();

bool isDeviceSuitable(VkPhysicalDevice device);

bool checkDeviceExtensionSupport(VkPhysicalDevice device);

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

VkSampleCountFlagBits getMaxUsableSampleCount();

void createLogicalDevice();

static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);