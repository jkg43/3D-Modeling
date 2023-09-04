#include "Constants.h"




void RenderObject::scale(float factor)
{
	verticesChanged = true;
	for (Vertex& v : vertices)
	{
		v.pos *= factor;
	}
}