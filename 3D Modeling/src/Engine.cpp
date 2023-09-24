#include "Constants.h"
#include "Engine.h"
#include "VulkanManager.h"
using namespace glm;

void initEngine()
{
	vg.engine.translateAxes.resize(3);

	vg.engine.vertexSelectionDisplay = &vg.renderObjects[4];
	vg.engine.vertexHoverDisplay = &vg.renderObjects[11];

	vg.engine.vertexSelectionDisplay->isVisible = false;
	vg.engine.vertexHoverDisplay->isVisible = false;

	float axisWidth = 0.05f;
	float axisLength = 0.25f;
	float axisOffset = 0.1f;

	loadCustomCube(vg.engine.translateAxes[0], axisLength, axisWidth, axisWidth, 1.0f, 0, 0,glm::vec3(axisOffset,0,0));
	loadCustomCube(vg.engine.translateAxes[1], axisWidth, axisLength, axisWidth, 0, 1.0f, 0,glm::vec3(0,axisOffset,0));
	loadCustomCube(vg.engine.translateAxes[2], axisWidth, axisWidth, axisLength, 0, 0, 1.0f, glm::vec3(0, 0, axisOffset));

	int i = 100;
	for (RenderObject &o : vg.engine.translateAxes)
	{
		createVertexBuffer(o, i);
		createIndexBuffer(o, i);
		i++;
	}

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