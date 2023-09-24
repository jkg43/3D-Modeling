#pragma once
#include "StandardIncludes.h"
#include "GeometryTypes.h"
#include "Transform.h"
#include "VulkanGlobals.h"
#include "WindowManager.h"
#include "ValidationLayers.h"
#include "Tools.h"
#include "VulkanManager.h"
#include "DeviceManager.h"
#include "InputManager.h"
#include "GUIManager.h"
#include "LogicManager.h"




#define NEAR_CLIPPING_DIST 0.1f
#define FAR_CLIPPING_DIST 100.0f



const uint32_t WIDTH = 1600;
const uint32_t HEIGHT = 900;

const std::string MODEL_PATH = "models/viking_room.obj";
const std::string TEXTURE_PATH = "textures/viking_room.png";

const int MAX_FRAMES_IN_FLIGHT = 2;

extern int currentFrame;

extern VulkanGlobals vg;

extern double mx, my;


#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};




const float pi = (float)3.14159265358979323846264338327950288;
const float twoPi = (float)6.28318530717958647692528676655900576;



//default heap size: 32 MiB
const int DEFAULT_HEAP_SIZE = 33554432;


//max time for a click
const float clickTimeSeconds = 0.15f;


