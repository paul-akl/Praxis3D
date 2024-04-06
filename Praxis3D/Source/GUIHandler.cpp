
//#include "imgui.cpp"

#include "GUIHandler.h"

ErrorCode GUIHandler::init()
{
	ErrorCode returnError = ErrorCode::Success;

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	m_io = &ImGui::GetIO();

	// Enable docking if it is set in the config
	if(Config::GUIVar().gui_docking_enabled)
		m_io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// Setup Dear ImGui style
	if(Config::GUIVar().gui_dark_style)
		ImGui::StyleColorsDark();
	else
		ImGui::StyleColorsClassic();

	auto windowHandle = WindowLocator::get().getSDLWindowHandle();
	auto glContextHandle = WindowLocator::get().getGLContextHandle();

	if(windowHandle != nullptr)
	{
		if(glContextHandle != nullptr)
		{
			std::string glslVersionString = "#version " + Utilities::toString(Config::engineVar().glsl_version);
			const char *glsl_version = glslVersionString.c_str();

			auto &imguiIO = ImGui::GetIO();

			// Store the default clipboard implementation
			auto defaultSetClipboardTextFn = imguiIO.SetClipboardTextFn;
			auto defaultGetClipboardTextFn = imguiIO.GetClipboardTextFn;
			auto defaultClipboardUserData = imguiIO.ClipboardUserData;

			// Setup Platform/Renderer backends
			ImGui_ImplSDL2_InitForOpenGL(windowHandle, glContextHandle);
			ImGui_ImplOpenGL3_Init(glsl_version);

			// Reset the clipboard functions to the default implementation,
			// because SDL clipboard functions are not suitable for multithreading
			// (freezes the process for about 10s when writing to clipboard)
			imguiIO.SetClipboardTextFn = defaultSetClipboardTextFn;
			imguiIO.GetClipboardTextFn = defaultGetClipboardTextFn;
			imguiIO.ClipboardUserData = defaultClipboardUserData;

			auto *m_defaultFont = imguiIO.Fonts->AddFontDefault();

			m_initialized = true;
		}
		else
			returnError = ErrorCode::GL_context_missing;
	}
	else
		returnError = ErrorCode::Window_handle_missing;

	return returnError;
}

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