#pragma once
#include "StandardIncludes.h"


class Camera
{
public:

	float dist;

	Transform transform;

	glm::vec3 targetPos;


	glm::mat4 arcballViewMatrix();

	glm::vec3 forward();
	glm::vec3 right();
	glm::vec3 up();
	glm::vec3 getPosition();

	float objectDist = 3.0f;

	Camera();

	void resetCam();

};