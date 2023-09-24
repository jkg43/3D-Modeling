#include "Constants.h"
#include "glm/gtx/norm.hpp"
#include <float.h>
using namespace glm;

//logic manager interacts with the engine to update the internal state

void updateLogicState()
{

	vg.renderObjects[5].transform.position = vg.cam.targetPos - vec3(0.05f);

	vec3 camPos = vg.cam.getPosition();

	float minDist = FLT_MAX;
		
	VertexSelection closestVertex;

	vec3 worldPos, deltaVec;


	for (ModelObject &o : vg.modelObjects)
	{
		vec3 objPos = o.ro->transform.position;

		float dist = 0;

		for (Vertex& v : o.ro->vertices)
		{
			worldPos = objPos + v.pos;

			deltaVec = worldPos - camPos;

			dist = length(dot(vg.camRay, deltaVec) * vg.camRay - deltaVec);

			if (dist < minDist)
			{
				minDist = dist;
				closestVertex = VertexSelection(&v, &o);
			}
		}

	}
	
	for (auto &[id, sv] : vg.engine.selectedVertices)
	{
		if (sv.vertex == closestVertex.vertex && sv.object == closestVertex.object)
		{
			closestVertex.id = sv.id;
		}
	}

	//none hovered
	if (minDist > 0.3f)
	{
		vg.engine.vertexHoverDisplay->isVisible = false;
		vg.engine.isVertexHovered = false;
	}
	else
	{
		vg.engine.vertexHoverDisplay->isVisible = true;
		vg.engine.vertexHoverDisplay->transform.position = closestVertex.getPos() - 
			vec3(vg.engine.selectionDisplayRadius);

		vg.engine.hoveredVertex = closestVertex;
		vg.engine.isVertexHovered = true;
	}

	int i = 0;

	vg.engine.isAxisHovered = false;

	if (vg.engine.displayTranslateAxes)
	{
		for (RenderObject &o : vg.engine.translateAxes)
		{
			if (rayCollideRenderObject(o, vg.cam.getPosition(), vg.camRay))
			{
				for (Vertex &v : o.vertices)
				{
					v.color[i] = 0.5f;
					vg.engine.isAxisHovered = true;
					vg.engine.hoveredAxis = i;
				}
			}
			else
			{
				for (Vertex &v : o.vertices)
				{
					v.color[i] = 1.0f;
				}
			}
			i++;
		}
	}


	//else
	//{
	//	vg.engine.hoveredVertex = closestVertex;
	//	vg.engine.hoveredObject = closestObject;
	//	vg.engine.isVertexHovered = true;
	//	if (vg.engine.isVertexSelected)
	//	{
	//		vg.renderObjects[4].transform.position = vg.engine.sv->pos + 
	//			vg.engine.selectedObject->ro->transform.position - vec3(0.05f);
	//	}
	//	else
	//	{
	//		vg.renderObjects[4].transform.position = vg.engine.hoveredVertex->pos + 
	//			vg.engine.hoveredObject->ro->transform.position - vec3(0.05f);
	//	}
	//}

	//if (vg.engine.isVertexSelected && vg.engine.selectedObject->ro->verticesChanged)
	//{
	//	vg.renderObjects[4].transform.position = vg.engine.sv->pos + 
	//		vg.engine.selectedObject->ro->transform.position - vec3(0.05f);
	//}

	///*vg.renderObjects[4].transform.position = vec3(vg.debugValues[1],
	//	vg.debugValues[2], vg.debugValues[3]) - vec3(0.05f) +
	//	vg.modelObjects[1].ro->transform.position;*/


	//vg.engine.displayTranslateAxes = vg.engine.isVertexSelected;
	//if (vg.engine.isVertexSelected)
	//{
	//	vg.engine.axesPos = vg.engine.sv->pos + 
	//		vg.engine.selectedObject->ro->transform.position;

	//	for (RenderObject &o : vg.engine.translateAxes)
	//	{
	//		o.verticesChanged = true;
	//	}
	//}

	//int i = 0;

	//vg.engine.isAxisHovered = false;

	//if (vg.engine.displayTranslateAxes)
	//{
	//	for (RenderObject &o : vg.engine.translateAxes)
	//	{
	//		if (rayCollideRenderObject(o, vg.cam.getPosition(), vg.camRay))
	//		{
	//			for (Vertex &v : o.vertices)
	//			{
	//				v.color[i] = 0.5f;
	//				vg.engine.isAxisHovered = true;
	//				vg.engine.hoveredAxis = i;
	//			}
	//		}
	//		else
	//		{
	//			for (Vertex &v : o.vertices)
	//			{
	//				v.color[i] = 1.0f;
	//			}
	//		}
	//		i++;
	//	}
	//}



	const std::vector<Vertex> &v = vg.renderObjects[9].vertices;

	//bool result = rayCollideTriangle(vg.cam.getPosition(), vg.camRay, v[0].pos, v[1].pos, v[2].pos);

	bool result = rayCollideRenderObject(*vg.modelObjects[1].ro, vg.cam.getPosition(), vg.camRay);

	if (result)
	{
		vg.debugValues[0] = 1.0f;
		for (int i = 0; i < vg.renderObjects[9].vertices.size(); i++)
		{
			vg.renderObjects[9].vertices[i].color = vec3(0, 1.0, 0);
		}
		vg.renderObjects[9].verticesChanged = true;
	}
	else
	{
		vg.debugValues[0] = 0.0f;
		for (int i = 0; i < vg.renderObjects[9].vertices.size(); i++)
		{
			vg.renderObjects[9].vertices[i].color = vec3(1.0, 0, 0);
		}
		vg.renderObjects[9].verticesChanged = true;
	}





}


void testUpdate()
{

}

