
#include "ErrorHandlerLocator.h"
#include "GUIHandler.h"
#include "Version.h"
#include "Window.h"

Window::Window()
{
	m_mouseInfo.clear();
	
	m_mouseCapturedBeforeLostFocus = false;
	m_enableGUI = false;
	m_changesQueued = false;
	m_inFullscreen = false;

	m_numDisplays = 0;

	m_mousePositionBeforeSetRelativeX = 0;
	m_mousePositionBeforeSetRelativeY = 0;

	m_SDLWindow = nullptr;
	m_GLContext = nullptr;
	m_guiHandler = nullptr;

	// Reserve the space, since we know the number of supported scancodes
	m_scancodeNames.reserve(Scancode::NumberOfScancodes);

	// Iterate over all scancodes and match them with their name, in unordered map
	for(int i = 0; i < Scancode::NumberOfScancodes; i++)
	{
		Scancode scanCode = static_cast<Scancode>(i);
		m_binds[i].m_scancode = scanCode;
		m_scancodeNames[GetString(scanCode)] = scanCode;
	}
}
Window::~Window()
{
	SDL_Quit();
}

ErrorCode Window::init()
{
	ErrorCode returnError = ErrorCode::Success;

	// Add the current version number to the window name
	Config::m_windowVar.name = Config::m_windowVar.name + " v" + PRAXIS3D_VERSION_STRING;

	auto sdlError = SDL_Init(SDL_INIT_VIDEO);

	if(sdlError < 0)
		returnError = ErrorCode::SDL_video_init_failed;
	else
	{
		// Get number of displays, check if the number is valid, and check if default display number is valid
		auto numDisplays = SDL_GetNumVideoDisplays();
		if(numDisplays < 0)
		{
			returnError = ErrorCode::Invalid_num_vid_displays;
		}
		else
		{
			if(numDisplays < Config::windowVar().default_display - 1)
			{
				Config::setWindowVar().default_display = 0;
			}

			// Get display mode with information about current screen resolution (among other things)
			SDL_DisplayMode displayMode;
			SDL_GetCurrentDisplayMode(Config::windowVar().default_display, &displayMode);

			ErrHandlerLoc::get().log(ErrorType::Info, ErrorSource::Source_Window, "Number of displays: " + Utilities::toString(numDisplays));
			ErrHandlerLoc::get().log(ErrorType::Info, ErrorSource::Source_Window, "Current display resolution: " + Utilities::toString(displayMode.w) + "x" + Utilities::toString(displayMode.h));
			ErrHandlerLoc::get().log(ErrorType::Info, ErrorSource::Source_Window, "Current display refresh rate: " + Utilities::toString(displayMode.refresh_rate + "Hz"));

			// Check if the resolution in config doesn't extend beyond current display resolution
			if(Config::windowVar().window_size_fullscreen_x > displayMode.w)
				Config::setWindowVar().window_size_fullscreen_x = displayMode.w;

			if(Config::windowVar().window_size_fullscreen_y > displayMode.h)
				Config::setWindowVar().window_size_fullscreen_y = displayMode.h;

			if(Config::windowVar().window_size_windowed_x > displayMode.w)
				Config::setWindowVar().window_size_windowed_x = displayMode.w;

			if(Config::windowVar().window_size_windowed_y > displayMode.h)
				Config::setWindowVar().window_size_windowed_y = displayMode.h;

			m_inFullscreen = Config::windowVar().fullscreen;

			// Set current resolution depending on whether fullscreen is on by default
			if(m_inFullscreen)
			{
				Config::setGraphicsVar().current_resolution_x = Config::windowVar().window_size_fullscreen_x;
				Config::setGraphicsVar().current_resolution_y = Config::windowVar().window_size_fullscreen_y;
			}
			else
			{
				Config::setGraphicsVar().current_resolution_x = Config::windowVar().window_size_windowed_x;
				Config::setGraphicsVar().current_resolution_y = Config::windowVar().window_size_windowed_y;
			}
		}
	}

	return returnError;
}

ErrorCode Window::createWindow()
{
	ErrorCode returnError = ErrorCode::Success;

	// Set OpenGL context versions (if the version is not supported, no context will be created)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, Config::engineVar().gl_context_major_version);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, Config::engineVar().gl_context_minor_version);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, Config::graphicsVar().double_buffering);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, Config::graphicsVar().alpha_size);

	// Turn on multi-sampling anti-aliasing (MSAA)
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, Config::graphicsVar().multisample_buffers);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, Config::graphicsVar().multisample_samples);

	// Assign the required window flags
	unsigned int windowFlags = SDL_WindowFlags::SDL_WINDOW_OPENGL | SDL_WindowFlags::SDL_WINDOW_SHOWN;

	// Set the border-less window flag (no window decorations)
	if(Config::windowVar().borderless)
		windowFlags |= SDL_WindowFlags::SDL_WINDOW_BORDERLESS;

	// Set the maximized window flag (spawn window maximized)
	if(Config::windowVar().maximized)
		windowFlags |= SDL_WindowFlags::SDL_WINDOW_MAXIMIZED;

	// Set the re-sizable window flag
	if(Config::windowVar().resizable)
		windowFlags |= SDL_WindowFlags::SDL_WINDOW_RESIZABLE;

	// Define window position to either a set value or a centered position
	const int windowPositionX = Config::windowVar().window_position_centered ? SDL_WINDOWPOS_CENTERED : Config::windowVar().window_position_x;
	const int windowPositionY = Config::windowVar().window_position_centered ? SDL_WINDOWPOS_CENTERED : Config::windowVar().window_position_y;

	// Spawn a window
	m_SDLWindow = SDL_CreateWindow(	
		Config::windowVar().name.c_str(),
		windowPositionX, 
		windowPositionY,
		Config::graphicsVar().current_resolution_x, 
		Config::graphicsVar().current_resolution_y, 
		windowFlags);

	// Check if the creation of the window was successful
	if(!m_SDLWindow)
		returnError = ErrorCode::Window_creation_failed;
	else
	{
		ErrHandlerLoc::get().log(ErrorType::Info, ErrorSource::Source_Window, "SDL window has been created");

		// Create OpenGL context and attach it to the window
		m_GLContext = SDL_GL_CreateContext(m_SDLWindow);

		setVerticalSync(Config::windowVar().vertical_sync);

		// If mouse cursor is clipped, we need to hide it
		setMouseRelativeMode(Config::windowVar().mouse_captured);
		
		// Set if the mouse should warm to screen center in relative capture mode
		if(Config::inputVar().mouse_warp_mode)
			SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1", SDL_HINT_OVERRIDE);
		else
			SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "0", SDL_HINT_OVERRIDE);
	}

	return returnError;
}

void Window::handleEvents()
{
	// Clear mouse value from the previous frame
	m_mouseInfo.clear();
	
	// Process queued up changes
	processChanges();

	SDL_Event SDLEvent;
	while(SDL_PollEvent(&SDLEvent))
	{
		// If QUIT event is received, shutdown the engine
		if(SDLEvent.type == SDL_QUIT)
		{
			Config::setEngineVar().running = false;
			break;
		}
		else
		{
			// If GUI is enabled, send the event to it
			// A fix for right mouse button being stuck down (in ImGui) when moving the camera around in the editor, which forces mouse capturing and in turn doesn't send the mouse up events to ImGui:
			// send the mouse button up events to ImGui even when mouse capturing is turned on
			if(m_enableGUI && (!Config::m_windowVar.mouse_captured || SDLEvent.type == SDL_MOUSEBUTTONUP))
				m_guiHandler->processSDLEvent(SDLEvent);
				
			handleSDLEvent(SDLEvent);
		}
	}

	// If filtering is enabled, interpolate mouse location over current and previous frame; otherwise use current frame data
	if(Config::inputVar().mouse_filter)
	{
		m_mouseInfo.m_movementX = (m_mouseInfo.m_movementCurrentFrameX + m_mouseInfo.m_movementPrevFrameX) * 0.5f;
		m_mouseInfo.m_movementY = (m_mouseInfo.m_movementCurrentFrameY + m_mouseInfo.m_movementPrevFrameY) * 0.5f;

		// Save the current frame's mouse location to be used for next frame
		m_mouseInfo.m_movementPrevFrameX = m_mouseInfo.m_movementCurrentFrameX;
		m_mouseInfo.m_movementPrevFrameY = m_mouseInfo.m_movementCurrentFrameY;
	}
	else
	{
		m_mouseInfo.m_movementX = (float)m_mouseInfo.m_movementCurrentFrameX;
		m_mouseInfo.m_movementY = (float)m_mouseInfo.m_movementCurrentFrameY;
	}
}

void Window::handleSDLEvent(const SDL_Event &p_SDLEvent)
{
	switch(p_SDLEvent.type)
	{
	case SDL_WINDOWEVENT:
		{
			switch(p_SDLEvent.window.event)
			{
				case SDL_WINDOWEVENT_RESIZED:

					if(!Config::windowVar().fullscreen || Config::windowVar().fullscreen_borderless)
					{
						Config::setGraphicsVar().current_resolution_x = p_SDLEvent.window.data1;
						Config::setGraphicsVar().current_resolution_y = p_SDLEvent.window.data2;

						if(!Config::windowVar().fullscreen)
						{
							Config::setWindowVar().window_size_windowed_x = p_SDLEvent.window.data1;
							Config::setWindowVar().window_size_windowed_y = p_SDLEvent.window.data2;
						}

						ErrHandlerLoc::get().log(ErrorType::Info, ErrorSource::Source_Window, "The window has been resized to " + Utilities::toString(p_SDLEvent.window.data1) + "x" + Utilities::toString(p_SDLEvent.window.data2));
					}

					break;

				case SDL_WINDOWEVENT_MOVED:

					Config::setWindowVar().window_position_x = p_SDLEvent.window.data1;
					Config::setWindowVar().window_position_y = p_SDLEvent.window.data2;

					break;

				case SDL_WINDOWEVENT_FOCUS_GAINED:

					Config::m_windowVar.window_in_focus = true;

					// If mouse was captured before focus, re-capture it
					if(m_mouseCapturedBeforeLostFocus)
					{
						// Recapture the mouse
						Config::m_windowVar.mouse_captured = m_mouseCapturedBeforeLostFocus;
						setMouseRelativeMode(m_mouseCapturedBeforeLostFocus);

						// Set flag to false, so that it will only be set to true if focus is lost
						m_mouseCapturedBeforeLostFocus = false;
					}

					break;

				case SDL_WINDOWEVENT_FOCUS_LOST:

					Config::m_windowVar.window_in_focus = false;

					// If mouse should be un-captured on lost focus and mouse is currently captured
					if(Config::windowVar().mouse_release_on_lost_focus && Config::windowVar().mouse_captured)
					{
						// Set the flag to true, so that it will be recaptured on gained focus
						m_mouseCapturedBeforeLostFocus = true;

						// Un-capture the mouse
						Config::m_windowVar.mouse_captured = false;
						setMouseRelativeMode(false);
					}

					break;

				case SDL_WINDOWEVENT_CLOSE:

					// This is not an ideal way to shutdown the engine, but if it happens,
					// mark engine as not running anymore, and it will be shutdown shortly

					Config::m_engineVar.running = false;

					break;
			}
			break;
		}

	case SDL_MOUSEMOTION:
		{
			if(!m_enableGUI || !m_guiHandler->isMouseCaptured())
			{
				// Get the relative mouse location
				m_mouseInfo.m_movementCurrentFrameX += p_SDLEvent.motion.xrel;
				m_mouseInfo.m_movementCurrentFrameY += p_SDLEvent.motion.yrel;
			}

			break;
		}

	case SDL_MOUSEWHEEL:
		{
			if(!m_enableGUI || !m_guiHandler->isMouseCaptured())
			{
				if(p_SDLEvent.wheel.x != 0)
				{
					m_mouseInfo.m_wheelX = p_SDLEvent.wheel.x;

					if(p_SDLEvent.wheel.x > 0)
						m_binds[Scancode::Mouse_wheelup].activate();
					else
						m_binds[Scancode::Mouse_wheeldown].activate();
				}
				else
				{
					m_mouseInfo.m_wheelY = p_SDLEvent.wheel.y;

					if(p_SDLEvent.wheel.y > 0)
						m_binds[Scancode::Mouse_wheelright].activate();
					else
						m_binds[Scancode::Mouse_wheelleft].activate();
				}
			}

			break;
		}

	case SDL_MOUSEBUTTONDOWN:
		{
			if(!m_enableGUI || !m_guiHandler->isMouseCaptured())
			{
				switch(p_SDLEvent.button.button)
				{
					case(SDL_BUTTON_LEFT):
						m_binds[Scancode::Mouse_left].activate();
						break;
					case(SDL_BUTTON_RIGHT):
						m_binds[Scancode::Mouse_right].activate();
						break;
					case(SDL_BUTTON_MIDDLE):
						m_binds[Scancode::Mouse_middle].activate();
						break;
					case(SDL_BUTTON_X1):
						m_binds[Scancode::Mouse_x1].activate();
						break;
					case(SDL_BUTTON_X2):
						m_binds[Scancode::Mouse_x2].activate();
						break;
				}
			}
			break;
		}

	case SDL_MOUSEBUTTONUP:
	{
		switch(p_SDLEvent.button.button)
		{
		case(SDL_BUTTON_LEFT) :
			m_binds[Scancode::Mouse_left].deactivate();
			break;
		case(SDL_BUTTON_RIGHT) :
			m_binds[Scancode::Mouse_right].deactivate();
			break;
		case(SDL_BUTTON_MIDDLE) :
			m_binds[Scancode::Mouse_middle].deactivate();
			break;
		case(SDL_BUTTON_X1) :
			m_binds[Scancode::Mouse_x1].deactivate();
			break;
		case(SDL_BUTTON_X2) :
			m_binds[Scancode::Mouse_x2].deactivate();
			break;
		}
		break;
	}

	case SDL_KEYDOWN:
		{
			if(!m_enableGUI || !m_guiHandler->isKeyboardCaptured())
			{
				// If the key's scancode is in the range of scancode enum, activate it
				if(p_SDLEvent.key.keysym.scancode < Scancode::NumberOfScancodes)
					m_binds[p_SDLEvent.key.keysym.scancode].activate();
			}
			break;
		}

	case SDL_KEYUP:
		{
			// If the key's scancode is in the range of scancode enum, deactivate it
			if(p_SDLEvent.key.keysym.scancode < Scancode::NumberOfScancodes)
				m_binds[p_SDLEvent.key.keysym.scancode].deactivate();

			break;
		}
	}
}

void Window::processChanges()
{
	// If any changes are queued up (none most of the time, so have an early check here)
	if(m_changesQueued)
	{
		// Iterate over all changes and process them
		for(decltype(m_changeQueue.size()) i = 0, size = m_changeQueue.size(); i < size; i++)
		{
			switch(m_changeQueue[i].getPropertyID())
			{
			case Properties::Fullscreen:

				// Get new value, if it differs from the current state, set the new mouse capture mode
				if(Config::windowVar().fullscreen != m_changeQueue[i].getBool())
				{
					// Set the new fullscreen flag
					Config::m_windowVar.fullscreen = m_changeQueue[i].getBool();

					// If it is switching to border-less fullscreen, set the fullscreen resolution
					if (Config::windowVar().fullscreen && !Config::windowVar().fullscreen_borderless)
					{
						setWindowSize(Config::windowVar().window_size_fullscreen_x, Config::windowVar().window_size_fullscreen_y);

						Config::setGraphicsVar().current_resolution_x = Config::windowVar().window_size_fullscreen_x;
						Config::setGraphicsVar().current_resolution_y = Config::windowVar().window_size_fullscreen_y;
					}
					
					// If it is switching to windowed, set the windowed resolution
					if(!Config::windowVar().fullscreen)
					{
						setWindowSize(Config::windowVar().window_size_windowed_x, Config::windowVar().window_size_windowed_y);

						Config::setGraphicsVar().current_resolution_x = Config::windowVar().window_size_windowed_x;
						Config::setGraphicsVar().current_resolution_y = Config::windowVar().window_size_windowed_y;
					}

					// Set the fullscreen mode
					setFullscreen(Config::windowVar().fullscreen);
				}

				break;

			case Properties::MouseCapture:

				// Get new value, if it differs from the current state, set the new mouse capture mode
				if(Config::windowVar().mouse_captured != m_changeQueue[i].getBool())
				{
					Config::m_windowVar.mouse_captured = m_changeQueue[i].getBool();
					setMouseRelativeMode(Config::windowVar().mouse_captured);
				}
				break;

			case Properties::VerticalSync:
				
				// Get new value, if it differs from the current state, update the vertical sync mode
				if(Config::windowVar().vertical_sync != m_changeQueue[i].getBool())
				{
					Config::m_windowVar.vertical_sync = m_changeQueue[i].getBool();
					setVerticalSync(Config::windowVar().vertical_sync);
				}
				break;

			case Properties::WindowTitle:

				// Change window title
				setWindowTitle(m_changeQueue[i].getString());

				break;
			}
		}

		// Clear the queued up changes
		m_changeQueue.clear();

		// Mark that all changes have been processed
		m_changesQueued = false;
	}
}
