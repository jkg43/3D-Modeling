#include "Constants.h"
#include "Engine.h"
#include "VulkanManager.h"
#include <float.h>
using namespace glm;

void Engine::initEngine()
{
	translateAxes.resize(3);

	vertexSelectionDisplay = vg.renderObjects[4];
	vertexHoverDisplay = vg.renderObjects[11];

	vertexSelectionDisplay->isVisible = false;
	vertexHoverDisplay->isVisible = false;

	float axisWidth = 0.05f;
	float axisLength = 0.25f;
	float axisOffset = 0.1f;

	loadCustomCube(&translateAxes[0], axisLength, axisWidth, axisWidth, 1.0f, 0, 0,glm::vec3(axisOffset,0,0));
	loadCustomCube(&translateAxes[1], axisWidth, axisLength, axisWidth, 0, 1.0f, 0,glm::vec3(0,axisOffset,0));
	loadCustomCube(&translateAxes[2], axisWidth, axisWidth, axisLength, 0, 0, 1.0f, glm::vec3(0, 0, axisOffset));

	int i = 100;
	for (RenderObject &o : translateAxes)
	{
		createVertexBuffer(&o, i);
		createIndexBuffer(&o, i);
		i++;
	}

	distToHoveredTriSquared = FLT_MAX;
	hoveredTri = TriSelection();

}

vec3 Engine::getMoveDirection(int axis)
{
	switch (axis)
	{
	case 0:
		return vec3(1, 0, 0);
	case 1:
		return vec3(0, 1, 0);
	case 2:
		return vec3(0, 0, 1);
	default:
		return vec3(0);
	}
}

vec3 Engine::getAxisDirection(int axis)
{
	switch (hoveredAxis)
	{
	case 0:
		return vec3(0, 1, 0);
	case 1:
		return vec3(1, 0, 0);
	case 2:
		return vg.cam.right();
	default:
		return vec3(0);
	}
}

RenderObject* Engine::newRenderObject()
{
	RenderObject* newRO = new RenderObject();
	
	vg.renderObjects.push_back(newRO);
	return newRO;
}

void Engine::createBuffers(RenderObject *o)
{
	static int i = 0;

	createVertexBuffer(o, i);
	createIndexBuffer(o, i);
	i++;
}