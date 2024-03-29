#pragma once
#include "StandardIncludes.h"

using namespace glm;

struct Vertex
{
	vec3 pos;
	vec3 color;
	vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}
	bool operator==(const Vertex& other) const
	{
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};

template<> struct std::hash<Vertex>
{
	size_t operator()(Vertex const& vertex) const
	{
		return ((hash<vec3>()(vertex.pos) ^
			(hash<vec3>()(vertex.color) << 1)) >> 1) ^
			(hash<vec2>()(vertex.texCoord) << 1);
	}
};


class Edge
{
public:
	Vertex *v1, *v2;

	Edge(Vertex *vert1, Vertex *vert2)
	{
		v1 = vert1;
		v2 = vert2;
	}

	Edge() {};

};


class Plane
{
public:
	std::vector<Vertex*> vertices;
	std::vector<Edge> edges;
	vec3 normal;

	void calculateNormal();

};


class Circle
{
public:
	vec3 pos;
	vec3 normal;
	float radius;

	Circle(vec3 position, vec3 norm, float rad) : pos{ position }, normal{ norm }, radius{ rad } {};
};





