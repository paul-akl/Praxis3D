
#include "GUIHandler.h"

void GUIHandler::render()
{
	//ImGui::ShowDemoWindow(&m_showDemoWindow);
	
	//ImGui::SetNextWindowSize(ImVec2(600.0f, 900.0f));
	// Create a window called "My First Tool", with a menu bar.
	//ImGui::Begin("My First Tool", &m_window1Open, ImGuiWindowFlags_MenuBar);

	//ImGui::Text("Hello, world %d", 123);

	//ImGui::End();

	//ImGui::Begin("My First Tool");
	//ImGui::Text("Hello, world");
	//ImGui::End();

	//glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	//glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
	//glClear(GL_COLOR_BUFFER_BIT);

	// Create draw data from GUI elements
	ImGui::Render();

	// Render the draw data to a framebuffer
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Prepare OpenGL states for a new frame here, since the "render" method can only be called from the rendering thread
	ImGui_ImplOpenGL3_NewFrame();

	// Frame is no longer ready to be rendered
	m_frameReady.clear();
}