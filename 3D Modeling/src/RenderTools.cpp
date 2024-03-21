#include "Constants.h"


void loadCube(RenderObject* o)
{
	o->vertices = {
		{{0.0f,0.0f,0.0f},{0.0f,0.0f,1.0f}},
		{{1.0f,0.0f,0.0f},{0.0f,0.0f,1.0f}},
		{{0.0f,1.0f,0.0f},{0.0f,0.0f,1.0f}},
		{{1.0f,1.0f,0.0f},{0.0f,0.0f,1.0f}},
		{{0.0f,0.0f,1.0f},{0.0f,0.0f,1.0f}},
		{{1.0f,0.0f,1.0f},{0.0f,0.0f,1.0f}},
		{{0.0f,1.0f,1.0f},{0.0f,0.0f,1.0f}},
		{{1.0f,1.0f,1.0f},{0.0f,0.0f,1.0f}}
	};

	o->indices = {
		2,1,0,1,2,3,
		4,5,6,7,6,5,
		0,1,5,5,4,0,
		6,2,0,0,4,6,
		3,2,6,6,7,3,
		5,1,3,3,7,5
	};
}

void loadCustomCube(RenderObject *o, float xScale, float yScale, float zScale, float r, float g, float b, glm::vec3 offset)
{
	o->vertices = {
		{{0.0f + offset.x,0.0f + offset.y,0.0f + offset.z},{r,g,b}},
		{{xScale + offset.x,0.0f + offset.y,0.0f + offset.z},{r,g,b}},
		{{0.0f + offset.x,yScale + offset.y,0.0f + offset.z},{r,g,b}},
		{{xScale + offset.x,yScale + offset.y,0.0f + offset.z},{r,g,b}},
		{{0.0f + offset.x,0.0f + offset.y,zScale + offset.z},{r,g,b}},
		{{xScale + offset.x,0.0f + offset.y,zScale + offset.z},{r,g,b}},
		{{0.0f + offset.x,yScale + offset.y,zScale + offset.z},{r,g,b}},
		{{xScale + offset.x,yScale + offset.y,zScale + offset.z},{r,g,b}}
	};

	o->indices = {
		2,1,0,1,2,3,
		4,5,6,7,6,5,
		0,1,5,5,4,0,
		6,2,0,0,4,6,
		3,2,6,6,7,3,
		5,1,3,3,7,5
	};
}

void loadCustomCube(RenderObject *o, float xScale, float yScale, float zScale, float r, float g, float b)
{
	loadCustomCube(o, xScale, yScale, zScale, r, g, b, glm::vec3(0));
}