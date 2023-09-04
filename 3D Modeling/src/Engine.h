#pragma once
#include "StandardIncludes.h"
#include "GeometryTypes.h"
#include "ModelObject.h"

//engine class for modeling systems
class Engine
{
public:

	bool isVertexHovered = false, isVertexSelected = false;

	Vertex *selectedVertex = nullptr, *hoveredVertex = nullptr;

	ModelObject *selectedObject = nullptr, *hoveredObject = nullptr;

	bool isPlaneHovered = false, isPlaneSelected = false;
	Plane hoveredPlane, selectedPlane;

	std::vector<RenderObject> translateAxes;
	std::vector<bool> isAxisHovered;

	bool displayTranslateAxes = false;
	glm::vec3 axesPos;

};

void initEngine();