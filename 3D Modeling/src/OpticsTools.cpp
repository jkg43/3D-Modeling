#include "Constants.h"

using namespace glm;



const float END_SIZE = 0.3f, CONNECTION_WIDTH = 0.1f;

const vec3 offset = vec3(END_SIZE / 2), connOffset = vec3(CONNECTION_WIDTH / 2, CONNECTION_WIDTH / 2, 0);


void createOpticalConnectionWithEnds(vec3 p1, vec3 p2)
{
	RenderObject *newConnection, *end1, *end2;
	newConnection = vg.engine.newRenderObject();
	end1 = vg.engine.newRenderObject();
	end2 = vg.engine.newRenderObject();
	loadCustomCube(end1, END_SIZE, END_SIZE, END_SIZE, 0, 0, 1.0f, p1 - offset);
	loadCustomCube(end2, END_SIZE, END_SIZE, END_SIZE, 0, 0, 1.0f, p2 - offset);
	float dist = distance(p1, p2);
	loadCustomCube(newConnection, CONNECTION_WIDTH, CONNECTION_WIDTH, -dist, 1.0f, 1.0f, 0);

	vec3 dir = normalize(p2 - p1);
	newConnection->transform.orientation = quatLookAt(dir, vec3(1, 0, 0));
	newConnection->transform.position = p1 - connOffset;

	vg.engine.createBuffers(newConnection);
	vg.engine.createBuffers(end1);
	vg.engine.createBuffers(end2);
}

void createOpticalConnection(vec3 p1, vec3 p2)
{
	RenderObject *newConnection;
	newConnection = vg.engine.newRenderObject();
	float dist = distance(p1, p2);
	//loadCustomCube(newConnection, CONNECTION_WIDTH, CONNECTION_WIDTH, -dist, 1.0f, 1.0f, 0);
	loadCylinder(newConnection, CONNECTION_WIDTH / 2, -dist, vec3(1, 1, 0));

	vec3 dir = normalize(p2 - p1);
	newConnection->transform.orientation = quatLookAt(dir, vec3(1, 0, 0));
	newConnection->transform.position = p1;

	vg.engine.createBuffers(newConnection);
}