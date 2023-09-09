#include "Constants.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <chrono>
#include "GeometryTypes.h"

double mx=0, my=0;

void initInputCallbacks()
{
	glfwSetKeyCallback(vg.window, keyCallback);
	glfwSetCursorPosCallback(vg.window, cursorPositionCallback);
	glfwGetCursorPos(vg.window, &mx, &my);
	glfwSetCursorEnterCallback(vg.window, cursorEnterCallback);
	glfwSetMouseButtonCallback(vg.window, mouseButtonCallback);
	glfwSetScrollCallback(vg.window, scrollCallback);
}

const float moveSpeed = 0.01f;
const float rotationSpeed = -0.005f;

bool draggingLeft = false, draggingRight = false;
double leftDragX = 0, leftDragY = 0, rightDragX = 0, rightDragY = 0;

//0 for move/rotate camera
//1 for moving an object along an axis
#define DRAG_TYPE_CAM 0
#define DRAG_TYPE_MOVE 1
int dragTypeLeft = 0, dragTypeRight = 0;
//direction of movement
glm::vec3 moveDirection = glm::vec3(0);
glm::vec3 axisDirection = glm::vec3(0);

//vars used for detecting single clicks

std::chrono::steady_clock::time_point clickStart;
bool inClick;
float clickTime;
int clickButton = -1;
//how much the mouse has moved since last frame
int deltaM = 0;
//only allow dragging before the click check time is up if the mouse has moved more than this
#define deltaMThreshold 6


void processInputs()
{
	ImGuiIO& guiIO = ImGui::GetIO();
	if (!guiIO.WantCaptureMouse)
	{

		clickTime = std::chrono::duration_cast<std::chrono::duration<float>>
			(std::chrono::steady_clock::now() - clickStart).count();



		if (draggingLeft)
		{
			double dx = mx - leftDragX, dy = my - leftDragY;
			float ax = (float)dx * rotationSpeed, ay = (float)dy * rotationSpeed;
			switch(dragTypeLeft)
			{
			case DRAG_TYPE_CAM:

				/*
				//float cosax = cos(ax), sinax = sin(ax);

				//vg.cam.transform.position = glm::vec3(pos.x * cosax - pos.y * sinax,
				//	pos.y * cosax + pos.x * sinax, vg.cam.transform.position.z);



				float dtheta = dx * rotationSpeed, dphi = dy * rotationSpeed;

				float cosdt = cos(dtheta), sindt = sin(dtheta), cosdp = cos(dphi), sindp = sin(dphi);

				float dist = sqrt(pos.x * pos.x + pos.y * pos.y);

				float invdist = 1 / dist;

				vg.cam.transform.position.x = vg.cam.targetPos.x + pos.x * cosdp * cosdt - pos.y * cosdp * sindt +
					pos.z * pos.x * invdist * sindp * cosdt - pos.z * pos.y * invdist * sindp * sindt;
				vg.cam.transform.position.y = vg.cam.targetPos.y + pos.y * cosdp * cosdt + pos.x * cosdp * sindt +
					pos.z * pos.y * invdist * sindp * cosdt + pos.z * pos.x * invdist * sindp * sindt;
				vg.cam.transform.position.z = vg.cam.targetPos.z + pos.z * cosdp - dist * sindp;

				glm::vec3 startAngles = glm::degrees(glm::eulerAngles(vg.cam.transform.orientation));

				vg.cam.transform.orientation = glm::quatLookAt(vg.cam.forward(), glm::vec3(0, 0, 1));

				glm::vec3 endAngles = glm::degrees(glm::eulerAngles(vg.cam.transform.orientation));

				if (abs(endAngles.z+180) - abs(startAngles.z+180) > 90)
				{
					std::cout << "Start angles: " << glm::to_string(startAngles) << ", End angle: "
						<< glm::to_string(endAngles) << std::endl;
					glm::vec3 p = vg.cam.transform.position;
					std::cout << "Theta: " << glm::degrees(atan(p.y / p.x)) << ", Phi: " <<
						glm::degrees(acos(p.z / sqrt(p.x * p.x + p.y + p.y + p.z * p.z))) << std::endl;
					//if (vg.cam.transform.position.z > 0)
					//{
					//	vg.cam.transform.position.x = vg.cam.targetPos.x;
					//	vg.cam.transform.position.y = vg.cam.targetPos.y;
					//	vg.cam.transform.position.z = vg.cam.targetPos.z + vg.cam.dist;
					//}
					//else
					//{
					//	vg.cam.transform.position.x = vg.cam.targetPos.x;
					//	vg.cam.transform.position.y = vg.cam.targetPos.y;
					//	vg.cam.transform.position.z = vg.cam.targetPos.z - vg.cam.dist;
					//}


				}

				//TODO decide if I want to use angle based(arcballViewMatrix) or
				//  position based(lookAt) for the camera
				//  angle based is simpler but less dynamic for non modeling applications
				//  position based is more general but more complicated


				//glm::quat delta = glm::quat(glm::vec3(ay, ax, 0));

				//glm::vec3 eulers = glm::eulerAngles(delta);

				//glm::quat o = glm::degrees(glm::eulerAngles(vg.cam.transform.orientation));

				//std::cout << "dx: " << dx << ", dy: " << dy << std::endl << "x: " << delta.x << ", y: "
				//	<< delta.y << ", z: " << delta.z << ", w: " << delta.w << std::endl;

				//std::cout << "Delta: " << glm::to_string(delta) << std::endl;

				//std::cout << "Camera: " << glm::to_string(vg.cam.transform.orientation) << std::endl;

				//std::cout << "Delta Pitch: " << eulers.x << ", Delta Yaw: " << eulers.y << ", Delta Roll: " << eulers.z
				//	<< std::endl;

				//std::cout << "Camera Pitch: " << o.x << ", Camera Yaw: " << o.y << ", Camera Roll: " << o.z
				//	<< std::endl;

				//vg.cam.transform.orientation *= delta;
				*/
				deltaM = (int)(dx + dy);

				if (deltaM > deltaMThreshold)
				{
					clickTime = 1;
				}

				if (clickTime > clickTimeSeconds)
				{
					glm::quat yawQuat = glm::angleAxis(ax, glm::vec3(0, 0, 1));
					glm::quat pitchQuat = glm::angleAxis(ay, glm::vec3(1, 0, 0));

					vg.cam.transform.orientation = yawQuat * vg.cam.transform.orientation * pitchQuat;
				}
				break;
			case DRAG_TYPE_MOVE:
				glm::vec3 moveAmount = movePointAlongLineFromRay(vg.engine.axesPos, moveDirection,
					axisDirection, vg.cam.getPosition(), getRayFromScreenPos(vec2(leftDragX, leftDragY)),
					getRayFromScreenPos(vec2(mx, my)));
				vg.engine.axesPos += moveAmount;
				if (vg.engine.isVertexSelected)
				{
					vg.engine.selectedVertex->pos += moveAmount;
					vg.engine.selectedObject->ro->verticesChanged = true;
				}
				break;
			}
			
			

				/*
				//https://stackoverflow.com/questions/17297374/adding-an-euler-angle-to-a-quaternion

				//o = glm::degrees(glm::eulerAngles(vg.cam.transform.orientation));

				//std::cout << "Camera after: " << glm::to_string(vg.cam.transform.orientation) << std::endl;

				//std::cout << "Camera Pitch: " << o.x << ", Camera Yaw: " << o.y << ", Camera Roll: " << o.z
				//	<< std::endl << std::endl;




				//arcballRotation(glm::vec2(leftDragX, leftDragY), glm::vec2(leftDragX + ax, leftDragY + ay),
				//	vg.cam.transform.orientation);

				//eulers = glm::eulerAngles(vg.cam.transform.orientation);
				//eulers += glm::vec3(ax, ay, 0);
				//vg.cam.transform.orientation = glm::quat(eulers);

				//float pitch = rotationSpeed * dy;
				//float yaw = rotationSpeed * dx;


				//glm::vec3 forward = vg.cam.forward();

				//glm::vec3 cameraUp = glm::rotate(vg.cam.up(), pitch, forward);
				//glm::vec3 cameraRight = glm::rotate(vg.cam.right(), yaw, forward);

				//vg.cam.transform.position += cameraUp;
				//vg.cam.transform.position += cameraRight;
				*/
			

			leftDragX = mx;
			leftDragY = my;
		}
		if (draggingRight)
		{
			double dx = mx - rightDragX, dy = my - rightDragY;
			switch (dragTypeRight)
			{
			case DRAG_TYPE_CAM:

				deltaM = (int)(dx + dy);

				if (deltaM > deltaMThreshold)
				{
					clickTime = 1;
				}

				if (clickTime > clickTimeSeconds)
				{
					glm::vec3 right = vg.cam.right();

					glm::vec3 delta = glm::vec3(dx * right.x * -moveSpeed,
						dx * right.y * -moveSpeed, dy * moveSpeed);

					vg.cam.targetPos += delta;
					vg.cam.transform.position += delta;
				}
				break;
			case DRAG_TYPE_MOVE:
				glm::vec3 moveAmount = movePointAlongLineFromRay(vg.engine.axesPos, moveDirection,
					axisDirection, vg.cam.getPosition(), getRayFromScreenPos(vec2(rightDragX, rightDragY)),
					getRayFromScreenPos(vec2(mx, my)));
				vg.engine.axesPos += moveAmount;
				if (vg.engine.isVertexSelected)
				{
					vg.engine.selectedVertex->pos += moveAmount;
					vg.engine.selectedObject->ro->verticesChanged = true;
				}
				break;
			}

			rightDragX = mx;
			rightDragY = my;
		}
	}


	vg.mousePosVector = glm::vec4((2 * mx / vg.swapChainExtent.width) - 1.0,
		(2 * my / vg.swapChainExtent.height) - 1.0f, 
		(NEAR_CLIPPING_DIST+vg.cam.objectDist) / (FAR_CLIPPING_DIST - NEAR_CLIPPING_DIST), 1.0f);


	vg.convertedMousePos = convertScreenVectorToWorldVector(vg.mousePosVector);

	vg.camRay = getRayFromScreenPos(glm::vec2(mx, my));


}


static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int modifiers)
{
	ImGuiIO& guiIO = ImGui::GetIO();
	if (!guiIO.WantCaptureKeyboard)
	{
		if (action == GLFW_PRESS)
		{
			switch (key)
			{
			case GLFW_KEY_SPACE:
				std::cout << glm::to_string(glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
					glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f))) << std::endl;
				break;
			case GLFW_KEY_B:
				for (MemBlock& b : vg.bufferMemManager.blocks)
				{
					std::cout << "Offset: " << b.offset << "\t Size: " << b.size << std::boolalpha << "\t Used: " << b.used
						<< "\t Name: " << b.name << std::endl;
				}
				std::cout << "Memory Used: " << (DEFAULT_HEAP_SIZE - vg.bufferMemManager.memoryLeft) / 1024 << " kB/" <<
					DEFAULT_HEAP_SIZE / 1024 << " kB" << std::endl;
				break;
			case GLFW_KEY_C:
				vg.bufferMemManager.combineBlocks();
				break;
			case GLFW_KEY_UP:
				//vg.cam.objectDist++;
				if (vg.engine.isVertexSelected)
				{
					vg.engine.selectedVertex->pos.z += 0.25;
					vg.engine.selectedObject->ro->verticesChanged = true;
				}
				break;
			case GLFW_KEY_DOWN:
				//vg.cam.objectDist--;
				if (vg.engine.isVertexSelected)
				{
					vg.engine.selectedVertex->pos.z -= 0.25;
					vg.engine.selectedObject->ro->verticesChanged = true;
				}
				break;
			case GLFW_KEY_M:
				std::cout << "Buffer Memory Left: " << vg.bufferMemManager.memoryLeft / 1024 << " kB" << std::endl;
				break;
			case GLFW_KEY_I:
				std::cout << std::boolalpha << "Wants mouse: " << guiIO.WantCaptureMouse <<
					", Wants keyboard: " << guiIO.WantCaptureKeyboard << std::noboolalpha << std::endl;
				break;
			case GLFW_KEY_D:
				vg.drawDebug = !vg.drawDebug;
				break;
			case GLFW_KEY_G:
				vg.drawDemo = !vg.drawDemo;
				break;
			case GLFW_KEY_V:
				std::cout << "WO1 Vert Count: " << vg.renderObjects[0].vertices.size();
				break;
			case GLFW_KEY_R:
				vg.cam.resetCam();
				break;
			case GLFW_KEY_LEFT_BRACKET:
				vg.modelObjects[0].shiftAlongNormal(0.2f, 1);
				break;
			case GLFW_KEY_RIGHT_BRACKET:
				vg.modelObjects[0].shiftAlongNormal(-0.2f, 1);
				break;
			case GLFW_KEY_MINUS:
				vg.modelObjects[0].shiftAlongNormal(0.2f, 2);
				break;
			case GLFW_KEY_EQUAL:
				vg.modelObjects[0].shiftAlongNormal(-0.2f, 2);
				break;
			case GLFW_KEY_SEMICOLON:
				vg.modelObjects[0].shiftAlongNormal(0.2f, 3);
				break;
			case GLFW_KEY_APOSTROPHE:
				vg.modelObjects[0].shiftAlongNormal(-0.2f, 3);
				break;
			case GLFW_KEY_F3:
				vg.debugValues[5] = 1.0f;
				break;
			default:
				break;
			}
		}
		else if (action == GLFW_RELEASE)
		{
			switch (key)
			{
			case GLFW_KEY_SPACE:
				break;
			default:
				break;
			}
		}
		else if (action == GLFW_REPEAT)
		{
			switch (key)
			{
			case GLFW_KEY_SPACE:
				break;
			case GLFW_KEY_UP:
				vg.cam.objectDist++;
				break;
			case GLFW_KEY_DOWN:
				vg.cam.objectDist--;
				break;
			case GLFW_KEY_LEFT_BRACKET:
				vg.modelObjects[0].shiftAlongNormal(0.2f, 1);
				break;
			case GLFW_KEY_RIGHT_BRACKET:
				vg.modelObjects[0].shiftAlongNormal(-0.2f, 1);
				break;
			case GLFW_KEY_MINUS:
				vg.modelObjects[0].shiftAlongNormal(0.2f, 2);
				break;
			case GLFW_KEY_EQUAL:
				vg.modelObjects[0].shiftAlongNormal(-0.2f, 2);
				break;
			case GLFW_KEY_SEMICOLON:
				vg.modelObjects[0].shiftAlongNormal(0.2f, 3);
				break;
			case GLFW_KEY_APOSTROPHE:
				vg.modelObjects[0].shiftAlongNormal(-0.2f, 3);
				break;
			default:
				break;
			}
		}
	}
}

static void cursorPositionCallback(GLFWwindow* window, double xPos, double yPos)
{
	mx = xPos;
	my = yPos;
}

static void cursorEnterCallback(GLFWwindow* window, int entered)
{
	if (entered)
	{
		//std::cout << "Cursor entered the window" << std::endl;
	}
	else
	{
		//std::cout << "Cursor left the window" << std::endl;
	}
}

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{

	if (action == GLFW_PRESS)
	{
		clickStart = std::chrono::steady_clock::now();
		inClick = true;
		clickButton = button;

		switch (button)
		{
		case GLFW_MOUSE_BUTTON_LEFT:
			draggingLeft = true;
			leftDragX = mx;
			leftDragY = my;
			if (vg.engine.isAxisHovered)
			{
				dragTypeLeft = DRAG_TYPE_MOVE;
				moveDirection = vg.engine.getMoveDirection(vg.engine.hoveredAxis);
				axisDirection = vg.engine.getAxisDirection(vg.engine.hoveredAxis);
			}
			else
			{
				dragTypeLeft = DRAG_TYPE_CAM;
			}
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			draggingRight = true;
			rightDragX = mx;
			rightDragY = my;
			if (vg.engine.isAxisHovered)
			{
				dragTypeRight = DRAG_TYPE_MOVE;
				moveDirection = vg.engine.getMoveDirection(vg.engine.hoveredAxis);
				axisDirection = vg.engine.getAxisDirection(vg.engine.hoveredAxis);
			}
			else
			{
				dragTypeRight = DRAG_TYPE_CAM;
			}
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:

			break;
		}
	}
	else if (action == GLFW_RELEASE)
	{
		if (inClick && clickButton == button)
		{
			std::chrono::duration<float> clickTime = std::chrono::duration_cast<std::chrono::duration<float>>
				(std::chrono::steady_clock::now() - clickStart);
			if (clickTime.count() <= clickTimeSeconds)
			{
				inClick = false;
				//printf("Click time: %fs\n", clickTime.count());
				mouseClickCallback(clickButton);
			}
		}


		switch (button)
		{
		case GLFW_MOUSE_BUTTON_LEFT:
			draggingLeft = false;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			draggingRight = false;
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:

			break;
		}
	}
}

static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
	ImGuiIO& guiIO = ImGui::GetIO();
	if (!guiIO.WantCaptureMouse)
	{
		vg.cam.dist *= static_cast<float>(pow(1.1, -yOffset));
	}
}

static void mouseClickCallback(int button)
{
	switch (button)
	{
	case GLFW_MOUSE_BUTTON_LEFT:
		//printf("LMB clicked\n");
		if (vg.engine.isVertexHovered)
		{
			if (vg.engine.isVertexSelected)
			{
				vg.engine.selectedVertex->color = glm::vec3(0, 0, 1.0f);
			}
			vg.engine.isVertexSelected = true;
			vg.engine.selectedVertex = vg.engine.hoveredVertex;
			vg.engine.selectedVertex->color = glm::vec3(1.0f, 0.6f, 0);
			vg.engine.selectedObject = vg.engine.hoveredObject;
			vg.engine.selectedObject->ro->verticesChanged = true;
		}
		else
		{
			if (vg.engine.isVertexSelected)
			{
				vg.engine.selectedVertex->color = glm::vec3(0, 0, 1.0f);
				vg.engine.selectedObject->ro->verticesChanged = true;
			}
			vg.engine.isVertexSelected = false;
		}
		break;
	case GLFW_MOUSE_BUTTON_RIGHT:
		//printf("RMB clicked\n");
		break;
	case GLFW_MOUSE_BUTTON_MIDDLE:
		//printf("MMB clicked\n");
		break;
	}
}