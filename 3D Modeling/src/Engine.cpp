#include "Constants.h"
#include "Engine.h"
#include "VulkanManager.h"


void initEngine()
{
	vg.engine.translateAxes.resize(3);
	vg.engine.isAxisHovered.resize(3);

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