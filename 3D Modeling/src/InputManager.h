#pragma once
#include "StandardIncludes.h"

void initInputCallbacks();
void processInputs();
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int modifiers);
static void cursorPositionCallback(GLFWwindow* window, double xPos, double yPos);
static void cursorEnterCallback(GLFWwindow* window, int entered);
static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);
static void mouseClickCallback(int button);