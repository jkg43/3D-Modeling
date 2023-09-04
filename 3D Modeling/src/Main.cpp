#include "Constants.h"
#include "Engine.h"
#include<iostream>


int main()
{
	try
	{
		initWindow();
		initVulkan();
		initImGUI();
		initEngine();
		mainLoop();
		cleanup();
	}
	catch (const std::exception e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}