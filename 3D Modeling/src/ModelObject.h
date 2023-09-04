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