#pragma once
#include "StandardIncludes.h"


class Transform
{
public:

	glm::vec3 position;

	glm::vec3 scale;

	glm::quat orientation;

	glm::mat4 getMatrix();

	Transform(glm::vec3 pos, glm::quat ori, glm::vec3 scl);

	Transform(glm::vec3 pos, glm::quat ori);

	Transform(glm::vec3 pos);

	Transform();
	
};