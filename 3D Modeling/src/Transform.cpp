#include "Constants.h"







glm::mat4 Transform::getMatrix()
{
	return glm::scale(scale) * glm::translate(position) * glm::mat4_cast(orientation);
}


Transform::Transform(glm::vec3 p, glm::quat o, glm::vec3 s)
{
	position = p;
	orientation = o;
	scale = s;
}

Transform::Transform(glm::vec3 p, glm::quat o) : Transform(p, o, glm::vec3(1, 1, 1)) {}

Transform::Transform(glm::vec3 p) : Transform(p, glm::quat(glm::vec3(0, 0, 0)), glm::vec3(1, 1, 1)) {}

Transform::Transform() : Transform(glm::vec3(0, 0, 0), glm::quat(glm::vec3(0, 0, 0)), glm::vec3(1, 1, 1)) {}


