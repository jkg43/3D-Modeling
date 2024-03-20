#include "Constants.h"
#include "RenderObject.h"
#include "ModelObject.h"
using namespace glm;

void loadModelObjectCube(ModelObject &o, RenderObject *ro, vec3 color)
{
	o.ro = ro;

	std::vector<Vertex> vertices =
	{
		{{0.0f,0.0f,0.0f},color},
		{{1.0f,0.0f,0.0f},color},
		{{0.0f,1.0f,0.0f},color},
		{{1.0f,1.0f,0.0f},color},
		{{0.0f,0.0f,1.0f},color},
		{{1.0f,0.0f,1.0f},color},
		{{0.0f,1.0f,1.0f},color},
		{{1.0f,1.0f,1.0f},color}
	};

	std::vector<uint32_t> indices =
	{
		2,1,0,1,2,3,
		4,5,6,7,6,5,
		0,1,5,5,4,0,
		6,2,0,0,4,6,
		3,2,6,6,7,3,
		5,1,3,3,7,5
	};

	uint32_t planeIndices[] = 
	{
		0,2,3,1,
		4,5,7,6,
		0,1,5,4,
		0,4,6,2,
		2,6,7,3,
		1,3,7,5
	};


	ro->vertices = vertices;
	ro->indices = indices;

	o.planes.resize(6);

	for (size_t i = 0; i < 6; i++)
	{
		o.planes[i].vertices.resize(4);
		o.planes[i].edges.resize(4);
		for (size_t j = 0; j < 4; j++)
		{
			o.planes[i].vertices[j] = &ro->vertices[planeIndices[i * 4 + j]];
			o.planes[i].edges[j] = Edge(&ro->vertices[planeIndices[i * 4 + j]], 
				&ro->vertices[planeIndices[i * 4 + (j + 1) % 4]]);
			
		}
		o.planes[i].calculateNormal();
	}

	
	o.loadIndices(planeIndices, 4);

}

void loadModelObjectCube(ModelObject &o, RenderObject *ro)
{
	loadModelObjectCube(o, ro, vec3(0, 0, 1));
}


void loadModelObjectCylinder(ModelObject &o, RenderObject *ro, float radius, float height, int subdivisions, vec3 color)
{

	o.ro = ro;

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


	o.planes.resize(2 + subdivisions);

	for (size_t i = 0; i < 2; i++)
	{
		o.planes[i].vertices.resize(subdivisions);
		o.planes[i].edges.resize(subdivisions);
		for (size_t j = 0; j < subdivisions; j++)
		{
			o.planes[i].vertices[j] = &ro->vertices[2 + j + subdivisions * i];
			o.planes[i].edges[j] = Edge(&ro->vertices[2 + j + subdivisions * i],
				&ro->vertices[2 + (j + 1) % subdivisions + subdivisions * i]);
		}
		o.planes[i].calculateNormal();
	}

	for (size_t i = 0; i < subdivisions; i++)
	{
		o.planes[i + 2].vertices.resize(4);
		o.planes[i + 2].edges.resize(4);

		o.planes[i + 2].vertices[0] = &ro->vertices[2 + subdivisions + i];
		o.planes[i + 2].vertices[1] = &ro->vertices[2 + i];
		o.planes[i + 2].vertices[2] = &ro->vertices[2 + (i + 1) % subdivisions];
		o.planes[i + 2].vertices[3] = &ro->vertices[2 + subdivisions + (i + 1) % subdivisions];

		for (size_t j = 0; j < 4; j++)
		{
			o.planes[i + 2].edges[j] = Edge(o.planes[i + 2].vertices[j], o.planes[i + 2].vertices[(j + 1) % 4]);
		}

		o.planes[i + 2].calculateNormal();
		
	}

	o.indices.resize(subdivisions * 2 + 4);
	for (size_t i = 0; i < subdivisions + 1; i++)
	{
		o.indices[i] = 2 + i % subdivisions;
		o.indices[subdivisions + i + 2] = subdivisions + 2 + i % subdivisions;
	}


	o.indices[subdivisions+1] = 0xFFFFFFFF;
	o.indices[subdivisions * 2 + 3] = 0xFFFFFFFF;

}

void loadModelObjectCylinder(ModelObject &o, RenderObject *ro, float radius, float height, int subdivisions)
{
	loadModelObjectCylinder(o, ro, radius, height, subdivisions, vec3(0, 1, 0));
}


void loadModelObjectSphere(ModelObject &o, RenderObject *ro, float radius, int subdivisions, vec3 color)
{
	o.ro = ro;

	std::vector<Vertex> vertices;

	int N = subdivisions;
	int H = N / 2;
	float R = radius;

	vertices.resize(2 + H * N);

	vertices[0] = { {0.0f,0.0f,R},color };
	vertices[1] = { {0.0f,0.0f,-R},color };

	float angle = 0, angleSpacing = twoPi / N;

	float deltaH = 2.0f * R / (H + 1); //maybe make it not linearly spaced and have more sections at top/bot

	std::vector<float> deltaHNonLinear(H);

	for (size_t i = 0; i < H; i++)
	{
		deltaHNonLinear[i] = 0; //TODO
	}

	for (size_t j = 0; j < N; j++)
	{
		float xDir = cos(angle);
		float yDir = sin(angle);

		float height = -R + deltaH;

		for (size_t i = 0; i < H; i++)
		{
			float sliceRadius = sqrt(R * R - height * height);
			vertices[2 + i * N + j] = { {xDir * sliceRadius,yDir * sliceRadius,height},color };
			height += deltaH;
		}

		angle += angleSpacing;
	}


	std::vector<uint32_t> indices;

	indices.resize(3 * 2 * H * N);

	for (size_t i = 0; i < H - 1; i++)
	{
		for (size_t j = 0; j < N; j++)
		{
			indices[6 * (i * N + j) + 0] = 2 + i * N + j;
			indices[6 * (i * N + j) + 1] = 2 + i * N + (j + 1) % N;
			indices[6 * (i * N + j) + 2] = 2 + (i + 1) * N + (j + 1) % N;
			indices[6 * (i * N + j) + 3] = 2 + i * N + j;
			indices[6 * (i * N + j) + 4] = 2 + (i + 1) * N + (j + 1) % N;
			indices[6 * (i * N + j) + 5] = 2 + (i + 1) * N + j;
		}
	}

	for (size_t j = 0; j < N; j++)
	{
		indices[6 * ((H - 1) * N + j) + 0] = 0;
		indices[6 * ((H - 1) * N + j) + 1] = 2 + (H - 1) * N + j;
		indices[6 * ((H - 1) * N + j) + 2] = 2 + (H - 1) * N + (j + 1) % N;
		indices[6 * ((H - 1) * N + j) + 3] = 1;
		indices[6 * ((H - 1) * N + j) + 4] = 2 + j;
		indices[6 * ((H - 1) * N + j) + 5] = 2 + (j + 1) % N;
	}

	ro->vertices = vertices;
	ro->indices = indices; 

}

