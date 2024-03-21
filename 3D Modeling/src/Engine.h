#pragma once
#include "StandardIncludes.h"
#include "GeometryTypes.h"
#include "ModelObject.h"

//engine class for modeling systems
class Engine
{
public:

	void initEngine();

	bool isVertexHovered = false;

	bool isPlaneHovered = false, isPlaneSelected = false;
	Plane hoveredPlane, selectedPlane;

	std::vector<RenderObject> translateAxes;

	bool isAxisHovered = false;
	int hoveredAxis = 0;

	bool displayTranslateAxes = false;
	glm::vec3 axesPos;

	glm::vec3 getMoveDirection(int axis);
	glm::vec3 getAxisDirection(int axis);

	std::unordered_map<size_t, VertexSelection> selectedVertices;

	std::unordered_map<size_t, TriSelection> selectedTris;

	RenderObject *vertexSelectionDisplay;
	float selectionDisplayRadius = 0.05f;
	RenderObject *vertexHoverDisplay;

	VertexSelection hoveredVertex;

	TriSelection hoveredTri;
	float distToHoveredTriSquared;

	RenderObject* newRenderObject();
	void createBuffers(RenderObject *o);

};
