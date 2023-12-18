#pragma once

#include <imgui.h>
#include <backends\imgui_impl_sdl2.h>
#include <backends\imgui_impl_opengl3.h>

#include "ErrorHandlerLocator.h"
#include "WindowLocator.h"

#include "ImGuizmo.h"

class GUIHandlerBase
{
	friend class Engine;
	friend class GUIPass;
	friend class GUIScene;
	friend class GUISystem;
	friend class GUITask;
public:
	GUIHandlerBase() : m_initialized(false) 
	{
		m_frameReady.clear();
	}

	// Let the GUI process an SDL event so it can be registered
	virtual void processSDLEvent(const SDL_Event &p_SDLEvent) { }

	// Has the GUI Handle been initialized
	const inline bool isInitialized() const { return m_initialized; }

	// Get an atomic flag that marks when the GUI frame is ready to be rendered
	inline std::atomic_flag &getFrameReadyFlag()  { return m_frameReady; }

	// Enable docking windows
	virtual void enableDocking() { }

	// Disable docking windows
	virtual void disableDocking() { }

	// Returns true if the mouse is captured by the GUI, thus mouse events shouldn't propagate to the main application
	virtual bool isMouseCaptured() { return false; }

	// Returns true if the keyboard is captured by the GUI, thus keyboard events shouldn't propagate to the main application
	virtual bool isKeyboardCaptured() { return false; }

protected:
	virtual ErrorCode init() { return ErrorCode::Success; }

	// Must be called ONCE upon startup, and only from the rendering thread
	virtual void initRendering() { }

	// Begin GUI frame; can be called from any thread
	virtual void beginFrame() { }

	// Render the GUI; must be called from the rendering thread
	virtual void render() { }

	// End GUI frame; can be called from any thread
	virtual void endFrame() { }

	bool m_initialized;
	std::atomic_flag m_frameReady;
};

class GUIHandlerNull : public GUIHandlerBase
{
	friend class Engine;
	friend class GUIScene;
	friend class GUISystem;
	friend class GUITask;
public:
	GUIHandlerNull() : GUIHandlerBase() { }
	~GUIHandlerNull() { }
};

class GUIHandler : public GUIHandlerBase
{
	friend class Engine;
	friend class GUIScene;
	friend class GUISystem;
	friend class GUITask;
public:
	GUIHandler() : GUIHandlerBase()
	{
		m_showDemoWindow = false;
		m_window1Open = false;
		m_io = nullptr;
	}
	~GUIHandler() 
	{    
		// Cleanup
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();

		delete m_io;
	}

	// Let the GUI process an SDL event so it can be registered
	void processSDLEvent(const SDL_Event &p_SDLEvent) 
	{
		ImGui_ImplSDL2_ProcessEvent(&p_SDLEvent);
	}

	void enableDocking()
	{
		m_io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	}

	void disableDocking()
	{
		m_io->ConfigFlags &= ~(ImGuiConfigFlags_DockingEnable);
	}

	// Returns true if the mouse is captured by the GUI, thus mouse events shouldn't propagate to the main application
	bool isMouseCaptured() { return m_io->WantCaptureMouse; }

	// Returns true if the keyboard is captured by the GUI, thus keyboard events shouldn't propagate to the main application
	bool isKeyboardCaptured() { return m_io->WantCaptureKeyboard; }

protected:
	ErrorCode init()
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

				// Setup Platform/Renderer backends
				ImGui_ImplSDL2_InitForOpenGL(windowHandle, glContextHandle);
				ImGui_ImplOpenGL3_Init(glsl_version);

				m_initialized = true;
			}
			else
				returnError = ErrorCode::GL_context_missing;
		}
		else
			returnError = ErrorCode::Window_handle_missing;

		return returnError;
	}

	void initRendering() 
	{
		ImGui_ImplOpenGL3_NewFrame();
	}

	void beginFrame() 
	{
		// Begin new SDL frame (get windows data from SDL)
		ImGui_ImplSDL2_NewFrame();

		// Begin new GUI frame (prepares frame to receive new GUI calls)
		ImGui::NewFrame();

		// Mark the beginning of ImGuizmo frame
		ImGuizmo::BeginFrame();
	}

	void render();

	void endFrame() 
	{
		// Mark the frame as ready to be rendered
		m_frameReady.test_and_set();

		// Notify any waiting threads that the frame is ready
		m_frameReady.notify_all();
	}

private:
	bool m_showDemoWindow;
	bool m_window1Open;
	ImGuiIO *m_io;
};