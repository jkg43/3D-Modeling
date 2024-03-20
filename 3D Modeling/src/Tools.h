#pragma once
#include "StandardIncludes.h"
#include "ModelObject.h"
using namespace glm;

//ImageTools.cpp
VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
void createImage(uint32_t width, uint32_t height, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
	VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);


//CommandTools.cpp
VkCommandBuffer beginSingleTimeCommands();
void endSingleTimeCommands(VkCommandBuffer commandBuffer);

//MemoryTools.cpp
uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
std::vector<char> readFile(const std::string& filename);
void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
	VkBuffer& buffer,std::string name);
void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
void createVertexBuffer(RenderObject* o, int i);
void createIndexBuffer(RenderObject* o, int i);
void createModelIndexBuffer(ModelObject &o, int i);

//MathTools.cpp

void arcballRotation(const glm::vec2& start, const glm::vec2& end, glm::quat& orientation);
glm::vec3 getArcballVector(const glm::vec2& point);
//converts a position in screen space to a position in world space
// @param vec: position in screen space x, y , z[0-1], 1.0
glm::vec4 convertScreenVectorToWorldVector(glm::vec4 vec);
//gets a ray away from the mouse pointer in world space
// @param pos: 2D position of the mouse in screen space
glm::vec3 getRayFromScreenPos(glm::vec2 pos);
//does a raycast to see if a ray collides with a triangle
bool rayCollideTriangle(vec3 rayPos, vec3 rayDir, vec3 p1, vec3 p2, vec3 p3);
//does a raycast, and returns the collision point if it collides, or a vec3 of FLT_MAX if not
glm::vec3 rayCollideTrianglePoint(glm::vec3 rayPos, glm::vec3 rayDir, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
//does a raycast to see if a ray collides with a render object
bool rayCollideRenderObject(RenderObject &o, glm::vec3 rayPos, glm::vec3 rayDir);
//Checks if a ray vertically upwards from p collides with the line between p1 and p2
bool rayCollideLine2D(glm::vec2 p, glm::vec2 p1, glm::vec2 p2);
//checks if a point is inside a triangle in 2D
bool checkIfPointInTriangle2D(glm::vec2 p, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3);
//finds the intersection point between a ray and a plane
//vec has value of FLT_MAX if no intersection point exists
//v1 and v2 are two vectors on the plane
glm::vec3 findIntersectionPointOfRayAndPlane(glm::vec3 rayOri, glm::vec3 rayDir, glm::vec3 planeOri,
	glm::vec3 v1, glm::vec3 v2);
//moves a point along a line starting at the point closest to ray1 in the axis direction
//  and ending at the point on the line closest to ray2 in the axis direction
//  that ray1 points to and ending at the point along the axis that
//  ray2 points to
//assumes both rays start at the same point
//returns a 0 vec if it is not successful
vec3 movePointAlongLineFromRay(vec3 lineOrigin, vec3 lineDir, vec3 axis, vec3 rayOrigin, vec3 ray1, vec3 ray2);
//projects a ray onto a line by moving it along an axis until they intersect
//vec has a value of FLT_MAX if no intersection exists
vec3 projectRayOntoLineInDirection(vec3 rayOrigin, vec3 rayDir, vec3 lineOrigin, vec3 lineDir, vec3 axis);
//gets the squared distance between two points
float distSqr(vec3 v1, vec3 v2);
//does a raycast to see if a ray collides with a render object, and updates the engine's hovered tri
bool rayCollideModelObject(ModelObject &o, vec3 rayPos, vec3 rayDir);


//ModelingTools.cpp

void loadModelObjectCube(ModelObject &o, RenderObject *ro, vec3 color);
void loadModelObjectCube(ModelObject& o, RenderObject* ro);
void loadModelObjectCylinder(ModelObject &o, RenderObject *ro, float radius, float height, int subdivisions, vec3 color);
void loadModelObjectCylinder(ModelObject &o, RenderObject *ro, float radius, float height, int subdivisions);
void loadModelObjectSphere(ModelObject &o, RenderObject *ro, float radius, int subdivisions, vec3 color);


//RenderTools.cpp

void loadCube(RenderObject *o);
void loadCustomCube(RenderObject *o, float xScale, float yScale, float zScale, float r, float g, float b);
void loadCustomCube(RenderObject *o, float xScale, float yScale, float zScale, float r, float g, float b, glm::vec3 offset);