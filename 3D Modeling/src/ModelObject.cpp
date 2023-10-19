#include "Constants.h"



size_t VertexSelection::idCounter = 1;
size_t TriSelection::idCounter = 1;

void ModelObject::shiftAlongNormal(float dist,int plane)
{
	glm::vec3 shift = dist * planes[plane].normal;


	for (Vertex* v : planes[plane].vertices)
	{
		//printf("x: %f, y: %f, z: %f\n", v->pos.x, v->pos.y, v->pos.z);
		v->pos += shift;
	}

	ro->verticesChanged = true;

}


void ModelObject::loadIndices(uint32_t indicesList[], uint32_t verticesPerPlane)
{
	indices.resize(planes.size() * (verticesPerPlane + 1));

	for (size_t i = 0; i < planes.size(); i++)
	{
		for (size_t j = 0; j < verticesPerPlane; j++)
		{
			indices.push_back(indicesList[i * verticesPerPlane + j]);
		}
		indices.push_back(0xFFFFFFFF);
	}
}

void ModelObject::loadIndices(uint32_t indicesList[], std::vector<uint32_t> verticesPerPlaneList)
{
	uint32_t vertexCount = 0;
	for (size_t i = 0; i < verticesPerPlaneList.size() / 2; i++)
	{
		for (size_t j = 0; j < verticesPerPlaneList[2 * i + 1]; j++)
		{
			vertexCount += verticesPerPlaneList[2 * i];
		}
	}

	
}

