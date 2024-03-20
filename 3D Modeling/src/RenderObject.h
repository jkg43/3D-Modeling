#pragma once
#include "StandardIncludes.h"
#include "Transform.h"
#include "GeometryTypes.h"

class RenderObject
{
public:
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	VkBuffer vertexBuffer;
	size_t numVerticesInBuffer = 0;
	VkBuffer indexBuffer;

	VkBuffer vertexStagingBuffer;

	void scale(float factor);

	bool verticesChanged = false;

	bool isVisible = true;

	Transform transform;


};