#pragma once

#include <imgui.h>
#include <backends\imgui_impl_sdl.h>
#include <backends\imgui_impl_opengl3.h>

#include "ErrorHandlerLocator.h"
#include "WindowLocator.h"

class GUIHandlerBase
{
	friend class Engine;
	friend class GUIPass;
	friend class GUIScene;
	friend class GUISystem;
	friend class GUITask;
public:
	GUIHandlerBase() : m_initialized(false) { }

	virtual void processSDLEvent(const SDL_Event &p_SDLEvent) { }

	const inline bool isInitialized() const { return m_initialized; }

protected:
	virtual ErrorCode init() { return ErrorCode::Success; }

	virtual void beginFrame() { }
	virtual void render() { }
	virtual void endFrame() { }

	bool m_initialized;
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
		m_showDemoWindow = true;
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

	void processSDLEvent(const SDL_Event &p_SDLEvent) 
	{
		ImGui_ImplSDL2_ProcessEvent(&p_SDLEvent);
	}

protected:
	ErrorCode init()
	{
		ErrorCode returnError = ErrorCode::Success;

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		m_io = &ImGui::GetIO(); //(void)m_io;

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
				auto glslVersionString = "#version " + Utilities::toString(Config::engineVar().glsl_version);
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

	void beginFrame() 
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
	}

	void render() 
	{
		ImGui::ShowDemoWindow(&m_showDemoWindow);
		ImGui::Render();
		//glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		//glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		//glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void endFrame() 
	{

	}

private:
	bool m_showDemoWindow;
	ImGuiIO *m_io;
};