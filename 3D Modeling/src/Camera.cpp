#include "Constants.h"
#include <glm/gtx/quaternion.hpp>
#include <cmath>
#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/matrix_decompose.hpp>


glm::mat4 Camera::arcballViewMatrix()
{
	//how far the camera is from the target
	glm::mat4 translate = glm::translate(glm::mat4(1), glm::vec3(0, 0, -dist));
	//the rotation of the camera
	glm::mat4 rotationMat = glm::toMat4(glm::inverse(transform.orientation));
	//the position of the target
	glm::mat4 targetTranslate = glm::translate(glm::mat4(1), -targetPos);

	glm::mat4 viewMatrix = translate * rotationMat * targetTranslate;
	return viewMatrix;
}

glm::vec3 Camera::forward()
{
	return glm::normalize(targetPos - getPosition());
}

glm::vec3 Camera::right()
{
	return glm::normalize(glm::cross(forward(), glm::vec3(0, 0, 1)));
}

glm::vec3 Camera::up()
{
	return glm::cross(right(), forward());
}

glm::vec3 Camera::getPosition()
{
	glm::vec3 eulers = glm::eulerAngles(transform.orientation);

	float d2 = dist * sin(eulers.x);

	glm::vec3 pos = glm::vec3(
		d2 * sin(eulers.z),
		-d2 * cos(eulers.z),
		dist * cos(eulers.x)
	) + targetPos;

	return pos;
}

void Camera::resetCam()
{
	dist = 5;

	targetPos = glm::vec3(0, 0, 0);

	transform = Transform(glm::vec3(0, dist, 0), glm::angleAxis(20.0f, glm::vec3(1, 0, 0)));
}


Camera::Camera()
{
	resetCam();
}

