
#include "GUIHandler.h"

void GUIHandler::render()
{
	// Create draw data from GUI elements
	ImGui::Render();

	// Render the draw data to a framebuffer
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Prepare OpenGL states for a new frame here, since the "render" method can only be called from the rendering thread
	ImGui_ImplOpenGL3_NewFrame();

	// Frame is no longer ready to be rendered
	m_frameReady.clear();
}