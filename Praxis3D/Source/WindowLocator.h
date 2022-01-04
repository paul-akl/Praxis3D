#pragma once

#include "Math.h"
#include "Window.h"

// Service locator provides global access to specific intanciated classes (i.e. services),
// but retains the ability to switch instances. Uses dependency injection pattern.
// This particular instance is using a wrappper around the service. Hence instead of giving
// direct access to the service, it only allows certain functions to be called (for safety reasons).
class WindowLocator
{
public:
	class WindowWrapperBase
	{
		friend class WindowLocator;
	public:
		virtual int getScreenWidth() const { return 0; }
		virtual int getScreenHeight() const { return 0; }
		virtual glm::ivec2 getScreenSize() const { return glm::ivec2(); }

		virtual void bindCommand(std::string &p_keyName, KeyCommand *p_command) { }
		virtual void bindCommand(Scancode p_scancode, KeyCommand *p_command) { }

		virtual void unbindCommand(std::string &p_keyName, KeyCommand *p_command) { }
		virtual void unbindCommand(Scancode p_scancode, KeyCommand *p_command) { }

		virtual void unbindAll(std::string &p_keyName) { }
		virtual void unbindAll(Scancode p_scancode) { }

		virtual Scancode getScancode(const std::string &p_keyName) { return Scancode::Key_Invalid; }

		// Spawns a simple message box with "yes" and "no" buttons. Block execution until a button is clicked
		virtual bool spawnYesNoInfoBox(std::string p_title, std::string p_message) { return false; }

		// Spawns a simple message box with "yes" and "no" buttons. Block execution until a button is clicked
		virtual bool spawnYesNoErrorBox(std::string p_title, std::string p_message) { return false; }

		// Spawns an error box with only an "ok" button
		virtual void spawnErrorBox(std::string p_title, std::string p_message) { }

		// Sets if the window should display the mouse cursor or hide it
		virtual void setMouseShow(bool p_show) { }

		// Makes the window go into or out of full screen mode
		virtual void setFullscreen(bool p_capture) { }

		// Sets the mode of mouse being captures inside the window
		virtual void setMouseCapture(bool p_capture) { }

		// Sets the vertical synchronization
		virtual void setVerticalSync(bool p_enabled) { }

		// Set the window title to the passed string
		virtual void setWindowTitle(const std::string &p_title) { }

		// Current info about mouse
		virtual const Window::MouseInfo &getMouseInfo() const { return m_mouseInfo; }

		// Get the handle to SDL Window; returns nullptr if it's not present
		virtual SDL_Window *getSDLWindowHandle() { return nullptr; }

		// Get the handle to GL Context; returns nullptr if it's not present
		virtual SDL_GLContext *getGLContextHandle() { return nullptr; }

		// Register a GUI handler base
		virtual void registerGUIHandler(GUIHandlerBase *p_guiHanlder) { }

	protected:
		WindowWrapperBase() : m_validWindow(false) { }

		bool m_validWindow;
		Window::MouseInfo m_mouseInfo;
	};

	class WindowWrapper : public WindowWrapperBase
	{
		friend class WindowLocator;
	public:
		int getScreenWidth() const { return m_window->getScreenWidth(); }
		int getScreenHeight() const { return m_window->getScreenHeight(); }
		glm::ivec2 getScreenSize() const { return m_window->getWindowSize(); }

		void bindCommand(std::string &p_keyName, KeyCommand *p_command) { m_window->bindCommand(p_keyName, p_command); }
		void bindCommand(Scancode p_scancode, KeyCommand *p_command) { m_window->bindCommand(p_scancode, p_command); }

		void unbindCommand(std::string &p_keyName, KeyCommand *p_command) { m_window->unbindCommand(p_keyName, p_command); }
		void unbindCommand(Scancode p_scancode, KeyCommand *p_command) { m_window->unbindCommand(p_scancode, p_command); }

		void unbindAll(std::string &p_keyName) { m_window->unbindAll(p_keyName); }
		void unbindAll(Scancode p_scancode) { m_window->unbindAll(p_scancode); }

		Scancode getScancode(const std::string &p_keyName) { return m_window->getScancode(p_keyName); }
		
		// Spawns a simple message box with "yes" and "no" buttons. Block execution until a button is clicked
		virtual bool spawnYesNoInfoBox(std::string p_title, std::string p_message) { return m_window->spawnYesNoInfoBox(p_title, p_message); }

		// Spawns a simple message box with "yes" and "no" buttons. Block execution until a button is clicked
		virtual bool spawnYesNoErrorBox(std::string p_title, std::string p_message) { return m_window->spawnYesNoErrorBox(p_title, p_message); }

		// Spawns an error box with only an "ok" button
		virtual void spawnErrorBox(std::string p_title, std::string p_message) { return m_window->spawnErrorBox(p_title, p_message); }

		// Sets if the window should display the mouse cursor or hide it
		void setMouseShow(bool p_show) { m_window->setMouseShow(p_show); }

		// Makes the window go into or out of full screen mode
		virtual void setFullscreen(bool p_capture)
		{
			// Create a new property and queue it as a change
			Property fullscreenProperty(Properties::Fullscreen, p_capture);
			m_window->queueChange(fullscreenProperty);
		}

		// Sets the mode of mouse being captures inside the window
		virtual void setMouseCapture(bool p_capture) 
		{
			// Create a new property and queue it as a change
			Property mouseCaptureProperty(Properties::MouseCapture, p_capture);
			m_window->queueChange(mouseCaptureProperty);
		}

		// Sets the vertical synchronization
		void setVerticalSync(bool p_enabled)
		{
			// Create a new property and queue it as a change
			Property mouseCaptureProperty(Properties::VerticalSync, p_enabled);
			m_window->queueChange(mouseCaptureProperty);
		}

		// Set the window title to the passed string
		virtual void setWindowTitle(const std::string &p_title)
		{
			// Create a new property and queue it as a change
			Property windowTitleProperty(Properties::WindowTitle, p_title);
			m_window->queueChange(windowTitleProperty);
		}

		// Current info about mouse
		const Window::MouseInfo &getMouseInfo() const { return m_window->getMouseInfo(); }

		// Get the handle to SDL Window
		SDL_Window *getSDLWindowHandle() { return m_window->getSDLWindowHandle(); }

		// Get the handle to GL Context
		SDL_GLContext *getGLContextHandle() { return m_window->getGLContextHandle(); }

		// Register a GUI handler base
		virtual void registerGUIHandler(GUIHandlerBase *p_guiHanlder) { m_window->registerGUIHandler(p_guiHanlder); }

	protected:
		WindowWrapper(Window *p_window) : m_window(p_window) { m_validWindow = true; }
		
		Window *m_window;
	};

	// Get the service
	inline static WindowWrapperBase &get() { return *m_windowWrapper; }

	// Initialize the service locator to use null services
	inline static ErrorCode init() { m_windowWrapper = &m_nullWindowWrapper; return ErrorCode::Success; }

	inline static void provide(Window *p_window)
	{
		if(p_window != nullptr)
		{
			if(m_windowWrapper->m_validWindow)
				delete m_windowWrapper;

			m_windowWrapper = new WindowWrapper(p_window);
		}
	}

private:
	static WindowWrapperBase *m_windowWrapper;
	static WindowWrapperBase m_nullWindowWrapper;
};