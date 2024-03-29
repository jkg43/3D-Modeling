#include "Constants.h"
#include "OpticalModeling.h"
#include <float.h>


using namespace glm;

OpticsGlobals og;

Lens::Lens(float rad, float len, vec3 pos, vec3 normalDir, float a0, float a1, float a2, float a3)
{
	vec3 norm = normalize(normalDir);

	radius = rad;
	length = len;
	this->a0 = a0;
	this->a1 = a1;
	this->a2 = a2;
	this->a3 = a3;

	obj = vg.engine.newRenderObject();
	loadCylinder(obj, rad, len, vec3(0.5f, 0.5f, 0.5f));
	obj->transform.position = pos;
	obj->transform.orientation = quatLookAt(-norm, vec3(1, 0, 0));
	vg.engine.createBuffers(obj);

	inFace = new Circle(pos, norm, rad);
	outFace = new Circle(pos + norm * len, norm, rad);
}





OpticalRay::OpticalRay(vec3 start, vec3 normDir)
{
	origin = start;

	Lens *hitLens = nullptr;
	float hitDistSqr = FLT_MAX;
	vec3 firstHitPos = vec3();

	for (Lens *l : og.lenses)
	{
		vec3 collision = rayCollideCircle(start, normDir, l->inFace);
		if (collision.x != FLT_MAX)
		{
			float dSqr = distSqr(start, collision);
			if (dSqr < hitDistSqr)
			{
				hitDistSqr = dSqr;
				hitLens = l;
				firstHitPos = collision;
			}
		}
	}

	if (hitLens != nullptr)
	{
		endpoint = firstHitPos;
		createOpticalConnection(origin, endpoint);

		vec3 deltaN = normalize(endpoint - hitLens->inFace->pos);

		vec3 newStart = endpoint + normDir * hitLens->length;

		//shift angle at 1:a0 ratio
		vec3 newNorm = normalize(normDir + deltaN * hitLens->a0);

		new OpticalRay(newStart, newNorm); 

	}
	else
	{
		OpticalRay(start, normDir, 20);
	}
}




