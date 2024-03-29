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

void loadCylinder(RenderObject *ro, float radius, float height, vec3 color, size_t subdivisions)
{
	std::vector<Vertex> vertices;

	vertices.resize(subdivisions * 2 + 2);

	vertices[0] = { {0.0f,0.0f,0.0f},color };
	vertices[1] = { {0.0f,0.0f,height},color };

	float angle = 0, angleSpacing = twoPi / subdivisions;

	for (size_t i = 0; i < subdivisions; i++)
	{
		float xPos = radius * cos(angle);
		float yPos = radius * sin(angle);
		vertices[i + 2] = { {xPos,yPos,0.0f},color };//lower is first half of data range
		vertices[subdivisions + i + 2] = { {xPos,yPos,height},color };//upper second

		angle += angleSpacing;
	}

	std::vector<uint32_t> indices;

	indices.resize(subdivisions * 4 * 3);

	for (size_t i = 0; i < subdivisions; i++)
	{

		//bottom face
		indices[3 * i] = 0;
		indices[3 * i + 1] = 2 + (i + 1) % subdivisions;
		indices[3 * i + 2] = 2 + i;

		//top face
		indices[3 * subdivisions + 3 * i] = 1;
		indices[3 * subdivisions + 3 * i + 1] = 2 + subdivisions + i;
		indices[3 * subdivisions + 3 * i + 2] = 2 + subdivisions + (i + 1) % subdivisions;

		//upper tri of quad
		indices[3 * 2 * subdivisions + 3 * i] = 2 + subdivisions + i;
		indices[3 * 2 * subdivisions + 3 * i + 1] = 2 + i;
		indices[3 * 2 * subdivisions + 3 * i + 2] = 2 + subdivisions + (i + 1) % subdivisions;

		//lower tri of quad
		indices[3 * 3 * subdivisions + 3 * i] = 2 + i;
		indices[3 * 3 * subdivisions + 3 * i + 1] = 2 + (i + 1) % subdivisions;
		indices[3 * 3 * subdivisions + 3 * i + 2] = 2 + subdivisions + (i + 1) % subdivisions;

	}

	ro->vertices = vertices;
	ro->indices = indices;
}

void loadCylinder(RenderObject *ro, float radius, float height, vec3 color)
{
	loadCylinder(ro, radius, height, color, 50);
}