#include "Constants.h"
#include <iostream>
#include <string>
using namespace ImGui;
using namespace glm;

void initImGUI()
{
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	
	if (vkCreateDescriptorPool(vg.device, &pool_info, nullptr, &vg.imguiPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create ImGUI descriptor pool");
	}


	// 2: initialize imgui library

	//this initializes the core structures of imgui
	auto context = CreateContext();

	SetCurrentContext(context);

	//this initializes imgui for SDL
	ImGui_ImplGlfw_InitForVulkan(vg.window,true);

	//this initializes imgui for Vulkan
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = vg.instance;
	init_info.PhysicalDevice = vg.physicalDevice;
	init_info.Device = vg.device;
	init_info.Queue = vg.graphicsQueue;
	init_info.DescriptorPool = vg.imguiPool;
	init_info.MinImageCount = MAX_FRAMES_IN_FLIGHT;
	init_info.ImageCount = MAX_FRAMES_IN_FLIGHT;
	init_info.MSAASamples = vg.msaaSamples;
	init_info.Subpass = 1;

	ImGui_ImplVulkan_Init(&init_info, vg.renderPass);

	//execute a gpu command to upload imgui font textures

	VkCommandBuffer temp = beginSingleTimeCommands();
	ImGui_ImplVulkan_CreateFontsTexture(temp);
	endSingleTimeCommands(temp);

	//clear font textures from cpu data
	ImGui_ImplVulkan_DestroyFontUploadObjects();


	vg.debugValues.resize(6);

	vg.debugValues[5] = 0.0f;
}


extern double mx, my;
vec3 p = vec3(0), p1 = vec3(0), p2 = vec3(0), p3 = vec3(0);
void showDebugMenu()
{
	if (!Begin("Debug Menu", nullptr, 0))
	{
		End();
		return;
	}

	Text("Frame Time: %.2f", vg.frameTime);
	Text("FPS: %d", (int)(1000.0f / vg.frameTime));


	Text("Ray Direction");
	InputFloat("px", &p.x);
	InputFloat("py", &p.y);
	InputFloat("pz", &p.z);
	Text("Plane Vector 1");
	InputFloat("p1x", &p1.x);
	InputFloat("p1y", &p1.y);
	InputFloat("p1z", &p1.z);
	Text("Plane Vector 2");
	InputFloat("p2x", &p2.x);
	InputFloat("p2y", &p2.y);
	InputFloat("p2z", &p2.z);
	Text("Plane Origin");
	InputFloat("p3x", &p3.x);
	InputFloat("p3y", &p3.y);
	InputFloat("p3z", &p3.z);



	//vec3 resultVec = findIntersectionPointOfRayAndPlane(vec3(0), p, p3, p1, p2);

	vec3 resultVec = vec3(vg.debugValues[1], vg.debugValues[2], vg.debugValues[3]);

	if (resultVec.x == FLT_MAX)
	{
		Text("Intersection Point: No Intersection");
	}
	else
	{
		Text("Intersection Point: %f, %f, %f", resultVec.x, resultVec.y, resultVec.z);
	}

	if (vg.debugValues[5] != 0.0f)
	{
		vg.debugValues[5] = 2.0f;
	}


	vec3 camPos = vg.cam.getPosition();

	Text("Cam Pos: x: %f, y: %f, z: %f", camPos.x, camPos.y, camPos.z);

	vec3 camRay = vg.camRay;

	Text("Cam Ray: x: %f, y: %f, z: %f", camRay.x, camRay.y, camRay.z);

	RenderObject* o = vg.renderObjects[9];

	Text("Indices size: %d", o->indices.size() / 3);

	int length = o->indices.size() / 3;

	for (int i = 0; i < length; i++)
	{
		Text("Triangle %d: ", i);

		for (int j = 0; j < 3; j++)
		{
			vec3 vPos = o->vertices[o->indices[3 * i + j]].pos;
			Text("Vertex %d: %f, %f, %f, Index: %d, Index Val: %d", j, vPos.x, vPos.y, vPos.z, 3 * i + j,
				o->indices[3 * i + j]);
		}
	}

	Text("Size: %d", o->vertices.size());

	int i = 0;
	for (Vertex &v : o->vertices)
	{
		Text("Vertex %d: %f, %f, %f", i, v.pos.x, v.pos.y, v.pos.z);
		i++;
	}

	/*
	vec3 ray = vg.camRay;
	vec3 p = vg.cam.getPosition();
	ImGui::Text("Normalized Cam Ray: x: %f, y: %f, z: %f", ray.x, ray.y, ray.z);



	quat o = vg.cam.transform.orientation;
	ImGui::Text("Mouse Pos: %d, %d", (int)mx, (int)my);
	vec3 a = degrees(eulerAngles(o));
	ImGui::Text("Camera Eulers: pitch: %f, yaw: %f, roll: %f", a.x, a.y, a.z);
	vec3 t = vg.cam.targetPos;
	ImGui::Text("Camera position: x: %f, y: %f, z: %f", p.x, p.y, p.z);
	ImGui::Text("Target position: x: %f, y: %f, z: %f", t.x, t.y, t.z);
	ImGui::Text("Camera orientation: x: %f, y: %f, z: %f, w: %f",o.x,o.y,o.z,o.w);

	vec3 f = vg.cam.forward();
	vec3 r = vg.cam.right();
	vec3 u = vg.cam.up();
	ImGui::Text("Camera forward vector: x: %f, y: %f, z: %f", f.x, f.y, f.z);
	ImGui::Text("Camera right vector: x: %f, y: %f, z: %f", r.x, r.y, r.z);
	ImGui::Text("Camera up vector: x: %f, y: %f, z: %f", u.x, u.y, u.z);

	ImGui::Text("Screen Width: %d, Screen Height: %d", vg.swapChainExtent.width, vg.swapChainExtent.height);


	vec4 c = vg.mousePosVector;

	ImGui::Text("Mouse Screen Position Vector: x: %f, y: %f, z: %f, w: %f", c.x, c.y, c.z, c.w);

	c = vg.convertedMousePos;

	ImGui::Text("Mouse World Position Vector: x: %f, y: %f, z: %f, w: %f", c.x, c.y, c.z, c.w);

	ImGui::Text("NDC Mouse Pos: x: %f, y: %f", (2 * mx / vg.swapChainExtent.width) - 1.0,
		(2 * my / vg.swapChainExtent.height) - 1.0f);

	ImGui::Text("World z: %f", vg.mousePosVector.z * (FAR_CLIPPING_DIST - NEAR_CLIPPING_DIST));

	ImGui::Text("Object distance: %f", vg.cam.objectDist);


	ray = vg.renderObjects[4].transform.position;

	ImGui::Text("Render Object 4 Position: x: %f, y: %f, z: %f", ray.x, ray.y, ray.z);

	ImGui::Text("Render Object 4 Transform Matrix:");

	if (ImGui::BeginTable("table1", 4, ImGuiTableFlags_Borders))
	{
		mat4 transform = vg.renderObjects[4].transform.getMatrix();
		for (int row = 0; row < 4; row++)
		{
			ImGui::TableNextRow();
			for (int column = 0; column < 4; column++)
			{
				ImGui::TableSetColumnIndex(column);

				ImGui::Text("%f", transform[column][row]);
			}
		}
		ImGui::EndTable();
	}

	ImGui::Text("projXView Matrix:");

	if (ImGui::BeginTable("table1", 4, ImGuiTableFlags_Borders))
	{
		mat4 transform = vg.ubo.projXview;
		for (int row = 0; row < 4; row++)
		{
			ImGui::TableNextRow();
			for (int column = 0; column < 4; column++)
			{
				ImGui::TableSetColumnIndex(column);

				ImGui::Text("%f", transform[column][row]);
			}
		}
		ImGui::EndTable();
	}

	ImGui::Text("Inverse projXView Matrix:");

	if (ImGui::BeginTable("table1", 4, ImGuiTableFlags_Borders))
	{
		mat4 transform = inverse(vg.ubo.projXview);
		for (int row = 0; row < 4; row++)
		{
			ImGui::TableNextRow();
			for (int column = 0; column < 4; column++)
			{
				ImGui::TableSetColumnIndex(column);

				ImGui::Text("%f", transform[column][row]);
			}
		}
		ImGui::EndTable();
	}

	*/

	End();
}


void showOpticsOverlay()
{
	ImGuiIO &io = GetIO();
	const ImGuiViewport *viewport = GetMainViewport();
	ImVec2 work_pos = viewport->WorkPos;
	ImVec2 work_size = viewport->WorkSize;
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav
		| ImGuiWindowFlags_NoMove;
	ImVec2 window_pos, window_pivot;
	window_pos.x = work_pos.x + work_size.x - 10.0f;
	window_pos.y = work_pos.y + 10.0f;
	window_pivot.x = 1.0f;
	window_pivot.y = 0.0f;
	SetNextWindowPos(window_pos, ImGuiCond_Always, window_pivot);


	static float x = 0, y = 0, z = 0;



	if (Begin("Test fixed", NULL, window_flags))
	{
		ImGui::Text("test");
		InputFloat("X", &x);
		InputFloat("Y", &y);
		InputFloat("Z", &z);
		if (SmallButton("New Cube at coords"))
		{
			printf("New unit Cube at x: %.2f, y: %.2f, z: %.2f\n", x, y, z);
			RenderObject *newCube = vg.engine.newRenderObject();
			loadCube(newCube);
			vg.engine.createBuffers(newCube);
			newCube->transform.position = { x,y,z };
		}

	}
	End();


}
