#include "Constants.h"




void Plane::calculateNormal()
{
	glm::vec3 a = vertices.at(1)->pos - vertices.at(0)->pos;
	glm::vec3 b = vertices.at(2)->pos - vertices.at(0)->pos;

	normal = glm::normalize(glm::cross(a, b));

}


