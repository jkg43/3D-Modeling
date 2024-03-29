#pragma once
#include "StandardIncludes.h"
#include "RenderObject.h"

using namespace glm;


class Lens
{
public:
	RenderObject *obj;

	float a0, a1, a2, a3;
	float radius, length;

	Circle *inFace, *outFace;

	Lens(float rad, float len, vec3 pos, vec3 normalDir, float a0, float a1, float a2, float a3);
};


class OpticalRay
{
public:
	vec3 origin, endpoint;
	vec3 norm;


	//creates an optical ray that goes a certain distance
	OpticalRay(vec3 start, vec3 normDir, float dist)
	{
		origin = start;
		norm = normDir;
		endpoint = start + norm * dist;
		createOpticalConnection(origin, endpoint);
	}

	//creates an optical ray that interacts with lenses to create more rays
	OpticalRay(vec3 start, vec3 normDir);

};



typedef struct OpticsGlobals
{
	std::vector<Lens*> lenses;



} OpticsGlobals;



