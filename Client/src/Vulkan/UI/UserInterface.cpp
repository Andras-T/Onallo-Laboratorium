#include "include/UserInterface.h"
namespace Client {

	void UserInterface::draw(Window& window, VkCommandBuffer& commandBuffer) {
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		createFPSCounter();
		createMenuBar(window);
		menuShortcuts(window);

		ImGui::Begin("Connection Window");
		static char ip_address[64] = "";
		// Create an input field for the IP address
		ImGui::InputText("IP Address", ip_address, IM_ARRAYSIZE(ip_address));

		// Create a "Connect" button
		if (ImGui::Button("Connect")) {
			// Handle the connection logic here
			;
		}

		// End of ImGui window
		ImGui::End();

		ImGui::Render();
		ImDrawData* draw_data = ImGui::GetDrawData();
		ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);
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