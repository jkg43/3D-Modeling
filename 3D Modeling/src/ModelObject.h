#pragma once
#include "StandardIncludes.h"
#include "RenderObject.h"
#include "GeometryTypes.h"

class ModelObject
{
public:



	RenderObject* ro;

	std::vector<Plane> planes;

	void shiftAlongNormal(float dist,int plane);

	void loadIndices(uint32_t indicesList[], uint32_t verticesPerPlane);

	//vector stores number of consecutive planes that have the same number of vertices
	//  stored in pairs, starting with the number of vertices and then the length of the run
	void loadIndices(uint32_t indicesList[], std::vector<uint32_t> verticesPerPlaneList);

	VkBuffer lineIndexBuffer;

	std::vector<uint32_t> indices;

};


//including here because linker being weird
struct VertexSelection
{
	Vertex *vertex;
	ModelObject *object;
	size_t id;

	static size_t idCounter;

	VertexSelection(Vertex *v, ModelObject *o) : vertex{ v }, object{ o }
	{
		id = idCounter;
		idCounter++;
	}

	VertexSelection() : vertex{ nullptr }, object{ nullptr }, id{ 0 } {}

	glm::vec3 getPos()
	{
		return vertex->pos + object->ro->transform.position;
	}

	friend bool operator == (const VertexSelection &v1, const VertexSelection &v2)
	{
		return v1.id == v2.id;
	}
};


template<> struct std::hash<VertexSelection>
{
	size_t operator()(VertexSelection const &vertex) const
	{
		return vertex.id;
	}
};


struct TriSelection
{
	Vertex *vertices[3];
	ModelObject *object;
	size_t id;

	static size_t idCounter;

	TriSelection(Vertex *v1, Vertex *v2, Vertex *v3, ModelObject *o) : object{ o }
	{
		vertices[0] = v1;
		vertices[1] = v2;
		vertices[2] = v3;
		id = idCounter;
		idCounter++;
	}

	TriSelection() : vertices{ nullptr }, object{ nullptr }, id{ 0 } {}

	//vertexIndex must be 0, 1 or 2
	glm::vec3 getPos(size_t vertexIndex)
	{
		return vertices[vertexIndex]->pos + object->ro->transform.position;
	}

	friend bool operator == (const TriSelection &v1, const TriSelection &v2)
	{
		return v1.id == v2.id;
	}
};


template<> struct std::hash<TriSelection>
{
	size_t operator()(TriSelection const &tri) const
	{
		return tri.id;
	}
};


