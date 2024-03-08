#include "include/UserInterface.h"
namespace Client {

	void UserInterface::draw(GLFWwindow* window, VkCommandBuffer& commandBuffer) {
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();


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

		ImGui::Render();
		ImDrawData* draw_data = ImGui::GetDrawData();
		ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);
	}

};