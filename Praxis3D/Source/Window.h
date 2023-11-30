#pragma once

#include <sdl\SDL.h>
#include <map>
#include <unordered_map>

#include "Config.h"
#include "ErrorHandlerLocator.h"
#include "KeyCommand.h"
#include "Math.h"
#include "PropertySet.h"
#include "Scancodes.h"
#include "SpinWait.h"

class GUIHandlerBase;

class Window
{
public:
	enum WindowChanges
	{
		set_mouseCapture,
		set_verticalSync,
		set_windowTitle
	};
	struct MouseInfo
	{
		MouseInfo()
		{
			m_movementCurrentFrameX = 0;
			m_movementCurrentFrameY = 0;
			m_movementPrevFrameX = 0;
			m_movementPrevFrameY = 0;
			m_wheelX = 0;
			m_wheelY = 0;

			m_movementX = 0.0f;
			m_movementY = 0.0f;
		}

		inline void clear()
		{
			// Clear the current values
			m_movementCurrentFrameX = 0;
			m_movementCurrentFrameY = 0;
			m_movementX = 0.0f;
			m_movementY = 0.0f;
			m_wheelX = 0;
			m_wheelY = 0;
		}

		int m_movementCurrentFrameX;
		int m_movementCurrentFrameY;
		int m_movementPrevFrameX;
		int m_movementPrevFrameY;
		int m_wheelX;
		int m_wheelY;

		float m_movementX;
		float m_movementY;
	};

	Window();
	~Window();

	ErrorCode init();

	ErrorCode createWindow();

	void handleEvents();

	inline void bindCommand(const std::string &p_keyName, KeyCommand *p_command)
	{
		// Early fail if the passed string is empty
		if(!p_keyName.empty())
		{
			// Look for the entry by the passed key name
			auto scancodeIterator = m_scancodeNames.find(p_keyName);

			// If key name was matched, bind the command, if not, do nothing
			if(scancodeIterator != m_scancodeNames.end())
				bindCommand(scancodeIterator->second, p_command);
		}
	}
	inline void bindCommand(const Scancode p_scancode, KeyCommand *p_command)
	{
		// Block calls from other threads
		SpinWait::Lock lock(m_spinMutex);

		// Bind the command by directly accessing the array
		m_binds[p_scancode].bind(p_command);
	}

	inline void unbindCommand(const std::string &p_keyName, KeyCommand *p_command)
	{
		// Early fail if the passed string is empty
		if(!p_keyName.empty())
		{
			// Look for the entry by the passed key name
			auto scancodeIterator = m_scancodeNames.find(p_keyName);

			// If key name was matched, unbind the command, if not, do nothing
			if(scancodeIterator != m_scancodeNames.end())
				unbindCommand(scancodeIterator->second, p_command);
		}
	}
	inline void unbindCommand(const Scancode p_scancode, KeyCommand *p_command)
	{
		// Unbind the command by directly accessing the array
		m_binds[p_scancode].unbind(p_command);
	}

	inline void unbindAll(const std::string &p_keyName)
	{
		// Early fail if the passed string is empty
		if(!p_keyName.empty())
		{
			// Look for the entry by the passed key name
			auto scancodeIterator = m_scancodeNames.find(p_keyName);

			// If key name was matched, unbind all commands, if not, do nothing
			if(scancodeIterator != m_scancodeNames.end())
				unbindAll(scancodeIterator->second);
		}
	}
	inline void unbindAll(const Scancode p_scancode)
	{
		// Unbind all commands by directly accessing the array
		m_binds[p_scancode].unbindAll();
	}

	inline Scancode getScancode(const std::string &p_keyName)
	{
		// Early fail if the passed string is empty
		if(!p_keyName.empty())
		{
			// Look for the entry by the passed key name
			auto scancodeIterator = m_scancodeNames.find(p_keyName);

			// If key name was matched, return it
			return scancodeIterator->second;
		}

		// If this point is reached, no scancode has been found by the key name, so return invalid key
		return Scancode::Key_Invalid;
	}

	// Spawns a simple message box with "yes" and "no" buttons. Block execution until a button is clicked
	bool spawnYesNoInfoBox(std::string p_title, std::string p_message)
	{
		// Define buttons
		const SDL_MessageBoxButtonData buttons[] = 
		{
			{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "no" },
			{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "yes" }
		};

		// Define colors
		const SDL_MessageBoxColorScheme colorScheme = 
		{
			{ /* .colors (.r, .g, .b) */
			  /* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
				{ 255,   0,   0 },
				/* [SDL_MESSAGEBOX_COLOR_TEXT] */
				{ 0, 255,   0 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
				{ 255, 255,   0 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
				{ 0,   0, 255 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
				{ 255,   0, 255 }
			}
		};

		// Define data for message box
		const SDL_MessageBoxData messageboxdata = 
		{
			SDL_MESSAGEBOX_INFORMATION, /* .flags */
			m_SDLWindow, /* .window */
			p_title.c_str(), /* .title */
			p_message.c_str(), /* .message */
			SDL_arraysize(buttons), /* .numbuttons */
			buttons, /* .buttons */
			&colorScheme /* .colorScheme */
		};

		// Define button ID (used to check which buttons was clicked)
		int buttonID = 0;

		// Spawn a message box, return false if it failed
		if(SDL_ShowMessageBox(&messageboxdata, &buttonID) < 0)
			return false;

		// Check which button was pressed and returned the status
		return buttonID != 0;
	}
	
	// Spawns a simple message box with "yes" and "no" buttons. Block execution until a button is clicked
	bool spawnYesNoErrorBox(std::string p_title, std::string p_message)
	{
		// Define buttons
		const SDL_MessageBoxButtonData buttons[] =
		{
			{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "no" },
			{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "yes" }
		};

		// Define colors
		const SDL_MessageBoxColorScheme colorScheme =
		{
			{ /* .colors (.r, .g, .b) */
			  /* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
				{ 255,   0,   0 },
				/* [SDL_MESSAGEBOX_COLOR_TEXT] */
				{ 0, 255,   0 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
				{ 255, 255,   0 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
				{ 0,   0, 255 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
				{ 255,   0, 255 }
			}
		};

		// Define data for message box
		const SDL_MessageBoxData messageboxdata =
		{
			SDL_MESSAGEBOX_ERROR, /* .flags */
			m_SDLWindow, /* .window */
			p_title.c_str(), /* .title */
			p_message.c_str(), /* .message */
			SDL_arraysize(buttons), /* .numbuttons */
			buttons, /* .buttons */
			&colorScheme /* .colorScheme */
		};

		// Define button ID (used to check which buttons was clicked)
		int buttonID = 0;

		// Spawn a message box, return false if it failed
		if(SDL_ShowMessageBox(&messageboxdata, &buttonID) < 0)
			return false;

		// Check which button was pressed and returned the status
		return buttonID != 0;
	}

	// Spawns an error box with only an "ok" button
	void spawnErrorBox(std::string p_title, std::string p_message)
	{
		// Define buttons
		const SDL_MessageBoxButtonData buttons[] =
		{
			{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "ok" }
		};

		// Define colors
		const SDL_MessageBoxColorScheme colorScheme =
		{
			{ /* .colors (.r, .g, .b) */
			  /* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
				{ 255,   0,   0 },
				/* [SDL_MESSAGEBOX_COLOR_TEXT] */
				{ 0, 255,   0 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
				{ 255, 255,   0 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
				{ 0,   0, 255 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
				{ 255,   0, 255 }
			}
		};

		// Define data for message box
		const SDL_MessageBoxData messageboxdata =
		{
			SDL_MESSAGEBOX_ERROR, /* .flags */
			m_SDLWindow, /* .window */
			p_title.c_str(), /* .title */
			p_message.c_str(), /* .message */
			SDL_arraysize(buttons), /* .numbuttons */
			buttons, /* .buttons */
			&colorScheme /* .colorScheme */
		};

		// Define button ID (used to check which buttons was clicked)
		int buttonID = 0;

		// Spawn a message box
		SDL_ShowMessageBox(&messageboxdata, &buttonID);
	}

	// Sets if the window should display the mouse cursor or hide it
	void setMouseShow(const bool p_show)
	{
		// SDL implementation of bool is "0" or "1", so we need to "convert" the regular bool
		SDL_ShowCursor(p_show ? SDL_ENABLE : SDL_DISABLE);
	}

	// Sets relative mouse mode (cursor becomes hidden and clipped inside the window)
	void setMouseRelativeMode(const bool p_relativeMode)
	{
		// If the mouse is going into relative mode, save the current mouse position for later
		// If the mouse is going out of relative mode, restore its position to what it was before going into relative mode
		if(p_relativeMode)
			SDL_GetMouseState(&m_mousePositionBeforeSetRelativeX, &m_mousePositionBeforeSetRelativeY);
		else
			SDL_WarpMouseInWindow(m_SDLWindow, m_mousePositionBeforeSetRelativeX, m_mousePositionBeforeSetRelativeY);

		// SDL implementation of bool is "0" or "1", so we need to "convert" the regular bool
		SDL_SetRelativeMouseMode(p_relativeMode ? SDL_TRUE : SDL_FALSE);

		//ErrHandlerLoc::get().log(ErrorType::Info, ErrorSource::Source_Window, p_relativeMode ? "Mouse captured" : "Mouse released");
	}

	// Sets the vertical synchronization
	void setVerticalSync(bool p_enabled)
	{
		// Set the swap interval (i.e. vertical sync)
		int error = SDL_GL_SetSwapInterval((p_enabled == true ? 1 : 0));

		// Check if it was successful
		if(error == 0)
		{
			// If successful, log an info message
			ErrHandlerLoc::get().log(ErrorType::Info, ErrorSource::Source_Window, p_enabled ?
									 "Vertical synchronization enabled"
									 : "Vertical synchronization disabled");
		}
		else
		{
			// If it failed, get an error string from SDL
			std::string SDLError = SDL_GetError();
			if(!SDLError.empty())
			{
				ErrHandlerLoc::get().log(ErrorType::Warning, ErrorSource::Source_Window, SDLError);
				SDL_ClearError();
			}
			// If string is empty, log a more generic error
			else
				ErrHandlerLoc::get().log(ErrorCode::SDL_vsync_failed);
		}
	}

	// Sets the size of the window
	void setWindowSize(const int &p_width, const int &p_height)
	{
		if(Config::graphicsVar().current_resolution_x != p_width || Config::graphicsVar().current_resolution_y != p_height)
		{
			Config::setGraphicsVar().current_resolution_x = p_width;
			Config::setGraphicsVar().current_resolution_y = p_height;

			SDL_SetWindowSize(m_SDLWindow, p_width, p_height);
		}
	}

	// Set the window title to the passed string
	void setWindowTitle(const std::string &p_title)
	{
		SDL_SetWindowTitle(m_SDLWindow, p_title.c_str());
	}

	// Makes the window go into or out of the fullscreen mode
	void setFullscreen(const bool p_fullscreen)
	{
		SDL_SetWindowFullscreen(m_SDLWindow, p_fullscreen ? Config::windowVar().fullscreen_borderless ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN : 0);
	}

	// Enable GUI callbacks
	void setEnableGUI(const bool p_enable) 
	{
		if(p_enable && m_guiHandler != nullptr)
			m_enableGUI = true;
		else
			m_enableGUI = false;
	}

	// Register a GUI handler base
	void registerGUIHandler(GUIHandlerBase *p_guiHanlder) { m_guiHandler = p_guiHanlder; }

	// Swap front and back screen buffers
	inline void swapBuffers()
	{
		SDL_GL_SwapWindow(m_SDLWindow);
	}

	// Current screen size (can be either windowed or full-screen)
	const inline int getScreenWidth() const { return Config::graphicsVar().current_resolution_x; }
	const inline int getScreenHeight() const { return Config::graphicsVar().current_resolution_y; }

	const inline glm::ivec2 getWindowSize() const
	{
		glm::ivec2 returnVec;
		SDL_GetWindowSize(m_SDLWindow, &returnVec.x, &returnVec.y);
		return returnVec;
	}

	const inline MouseInfo &getMouseInfo() const { return m_mouseInfo; }

	SDL_Window *getSDLWindowHandle() { return m_SDLWindow; }
	SDL_GLContext *getGLContextHandle() { return &m_GLContext; }

	// Queues a change to be dealt with later (when handling events). Thread-safe.
	// Some changes require to be made on the main thread (one that created the window context),
	// hence why queuing is in order, and the changes are dealt with later, during event handling.
	inline void queueChange(Property &p_property)
	{
		// Lock the mutex before adding the property to a list
		SpinWait::Lock lock(m_changeQueueMutex);

		// Queue a change
		m_changeQueue.push_back(p_property);

		// Mark that there are changes queued up
		m_changesQueued = true;
	}

private:
	enum MouseBindingIndex : unsigned int
	{
		Mouse_left_index = 0,
		Mouse_middle_index,
		Mouse_right_index,
		Mouse_x1_index,
		Mouse_x2_index,
		Mouse_wheelup_index,
		Mouse_wheeldown_index,
		Mouse_wheelleft_index,
		Mouse_wheelright_index
	};

	// TODO: Add mutex if bind calls could happen from multiple threads
	struct Binds
	{
		Binds() : m_scancode(Scancode::Key_Invalid), m_numBinds(0) { }
		Binds(Scancode p_scanCode) : m_scancode(p_scanCode), m_numBinds(0) { }

		inline void activate()
		{
			decltype(m_numBinds) i = 0;
			for(; i < m_numBinds; i++)
				(*m_binds[i]).activate();
		}
		inline void deactivate()
		{
			decltype(m_numBinds) i = 0;
			for(; i < m_numBinds; i++)
				(*m_binds[i]).deactivate();
		}
		inline void bind(KeyCommand *p_command)
		{
			// Add the command to the binds array and update the array size
			m_binds.push_back(p_command);
			m_numBinds = m_binds.size();

			// Add the scancode to the command, so it internally knows that it has been bound to it
			p_command->addScancode(m_scancode);
		}
		inline void unbind(KeyCommand *p_command)
		{
			// Iterate over all binds, and match the passed command.
			// If the passed command is found, erase it, update bind array size and update the command on removal
			for(decltype(m_numBinds) i = 0; i < m_numBinds; i++)
				if((*m_binds[i]) == *p_command)
				{
					m_binds.erase(m_binds.begin() + i);
					m_numBinds = m_binds.size();

					// Remove the scancode form the command, so it internally know that it is not bound to it anymore
					p_command->removeScancode(m_scancode);
				}

		}
		inline void unbindAll()
		{
			// Notify each command on the scancode removal
			for(decltype(m_numBinds) i = 0; i < m_numBinds; i++)
				m_binds[i]->removeScancode(m_scancode);

			// Clear the bindings (but don't force to clearance of capacity, 
			// in case more commands to be bind later); update the bind array size
			m_binds.clear();
			m_numBinds = m_binds.size();
		}

		const inline bool operator<(const Scancode p_scanCode) { return (m_scancode < p_scanCode);		}
		const inline bool operator<(const Binds &p_binds) { return (m_scancode < p_binds.m_scancode);	}

		const inline bool operator==(const Scancode p_scanCode) { return (m_scancode == p_scanCode);	}
		const inline bool operator==(const Binds &p_binds) { return (m_scancode == p_binds.m_scancode); }

		std::vector<KeyCommand*> m_binds;
		std::vector<KeyCommand*>::size_type m_numBinds;

		Scancode m_scancode;
	};

	// Gets called on each SDL event, and process it
	void handleSDLEvent(const SDL_Event &p_SDLEvent);
	
	void processChanges();

	// Positions the mouse to the center of the window
	void warpMouseToScreenCenter()
	{
		SDL_WarpMouseInWindow(m_SDLWindow, Config::graphicsVar().current_resolution_x / 2, Config::graphicsVar().current_resolution_y / 2);
	}

	int m_numDisplays;

	bool m_mouseCapturedBeforeLostFocus;
	bool m_inFullscreen;

	int m_mousePositionBeforeSetRelativeX;
	int m_mousePositionBeforeSetRelativeY;

	// Handles to window and OpenGL contexts
	SDL_Window *m_SDLWindow;
	SDL_GLContext m_GLContext;

	// Hold current mouse data
	MouseInfo m_mouseInfo;

	// A command bind array for each supported button
	Binds m_binds[NumberOfScancodes];

	// Mutex used when queuing changes
	SpinWait m_changeQueueMutex;

	// Flag that tells if any changes are queued up, to be able to early bail if none are queued
	bool m_changesQueued;

	// Flag that tells if the GUI should be enabled (sending events to GUI and such)
	bool m_enableGUI;

	// A pointer to a GUI handler
	GUIHandlerBase *m_guiHandler;

	// Holds queued changes, in a form of properties
	std::vector<Property> m_changeQueue;

	// Ties scancodes and their names together, so we could match a key by it's name
	std::unordered_map<std::string, Scancode> m_scancodeNames;

	// Mutex used for multithreaded calls
	SpinWait m_spinMutex;
};