#include "include/UserInterface.h"
#include <Vulkan/Utils/ImVecUtils.h>

namespace Client {
	
	ImGuiStyle default_style;

	void UserInterface::draw(Window& window, VkCommandBuffer& commandBuffer, Input& uiInput) {

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_WindowBg].w = 0.1f;
		style.Colors[ImGuiCol_TitleBg].w = 0.25f;
		
		ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
		ImGui::SetNextWindowSizeConstraints(ImVec2(300.0f, 80.0f), ImVec2(400 , 120));
		connectionWindow(uiInput);
		createFPSCounter();
		createMenuBar(window);
		menuShortcuts(window);

		ImGui::Render();
		ImDrawData* draw_data = ImGui::GetDrawData();
		ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);
	}

	void UserInterface::connectionWindow(Input& uiInput) {
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowRounding = 7.0f;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		
		ImGui::Begin("Connection Window", nullptr);

		style.Colors[ImGuiCol_WindowBg].w = default_style.Colors[ImGuiCol_WindowBg].w;

		if (!uiInput.connected)
		{
			ImGui::PushItemWidth(200.0f);
			ImGui::InputText("IP Address", uiInput.ip_address, IM_ARRAYSIZE(uiInput.ip_address));
			ImGui::PopItemWidth();

			if (ImGui::Button("Connect"))
				uiInput.tryToConnect = true;

		}
		else {
			if (ImGui::Button("Disconnect"))
				uiInput.disconnect = true;
		}

		ImGui::End();
		ImGui::PopStyleVar(1);
	}

	void UserInterface::createFPSCounter()
	{

		ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_MenuBar;

		float height = ImGui::GetFrameHeight();

		if (ImGui::BeginViewportSideBar("Bottom bar", viewport, ImGuiDir_Down, height,
			window_flags)) {
			if (ImGui::BeginMenuBar()) {
				ImGui::Text("Status: ");
				ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0 / ImGui::GetIO().Framerate,
					ImGui::GetIO().Framerate);

				ImGui::EndMenuBar();
			}
			ImGui::End();
		}
	}

	void UserInterface::createMenuBar(Window& window)
	{
		ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_MenuBar;
		float height = ImGui::GetFrameHeight();

		if (ImGui::BeginViewportSideBar("Top bar", viewport, ImGuiDir_Up, height,
			window_flags)) {
			if (ImGui::BeginMenuBar()) {
				if (ImGui::BeginMenu("Menu")) {
					if (ImGui::MenuItem("Change display mode", "Alt+Enter"))
						window.changeDisplayMode();
					if (ImGui::MenuItem("Exit", "Alt+Q")) {
						glfwSetWindowShouldClose(window.get_GLFW_Window(), true);
					}
					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
			}
			ImGui::End();
		}
	}
	void UserInterface::menuShortcuts(Window& window)
	{
		if (ImGui::GetIO().KeyMods == ImGuiModFlags_Alt &&
			ImGui::IsKeyPressed(ImGuiKey_Enter, ImGuiKeyOwner_Any))
			window.changeDisplayMode();
	}
};