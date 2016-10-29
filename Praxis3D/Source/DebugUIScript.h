#pragma once

#include "BaseScriptObject.h"
#include "ClockLocator.h"
#include "KeyCommand.h"

// Provides bare user interface functionality, like button presses to toggle features (for debugging)
class DebugUIScript : public BaseScriptObject
{
public:
	DebugUIScript(SystemScene *p_systemScene, std::string p_name)
		: BaseScriptObject(p_systemScene, p_name, Properties::DebugUIScript)
	{

		m_elapsedTime = 0.0f;
		m_showFPSInterval = 1.0f;
	}
	~DebugUIScript() { }


	virtual ErrorCode init()
	{
		return ErrorCode::Success;
	}

	void loadToMemory()
	{

	}

	// Exports all the data of the object as a PropertySet
	virtual PropertySet exportObject()
	{
		// Create the root property set
		PropertySet propertySet(Properties::ArrayEntry);

		// Add variables
		propertySet.addProperty(Properties::Type, Properties::DebugUIScript);
		propertySet.addProperty(Properties::Name, m_name);

		// Add root key-binding property set
		auto &keyBinds = propertySet.addPropertySet(Properties::Keybindings);

		// Add individual key-bindings
		keyBinds.addProperty(Properties::DebugCaptureMouseKey, (int)m_mouseCaptureCommand.getFirstBinding());
		keyBinds.addProperty(Properties::DebugFullscreenKey, (int)m_fullscreenCommand.getFirstBinding());
		keyBinds.addProperty(Properties::DebugVertSyncKey, (int)m_vertSyncCommand.getFirstBinding());
		keyBinds.addProperty(Properties::CloseKey, (int)m_closeWindowCommand.getFirstBinding());

		return propertySet;
	}

	virtual void update(const float p_deltaTime)
	{
		if(m_closeWindowCommand.isActivated())
		{
			// Mark the state of the engine as not running anymore, and it will be shut down after the current frame
			Config::m_engineVar.running = false;
		}

		if(m_fullscreenCommand.isActivated())
		{
			// Toggle the fullscreen mode
			WindowLocator::get().setFullscreen(!Config::windowVar().fullscreen);

			// Deactivate the key command so this piece of code is not triggered again if the key wasn't released
			m_fullscreenCommand.deactivate();
		}

		if(m_mouseCaptureCommand.isActivated())
		{
			// Toggle the mouse capture mode
			WindowLocator().get().setMouseCapture(!Config::windowVar().mouse_captured);

			// Deactivate the key command so this piece of code is not triggered again if the key wasn't released
			m_mouseCaptureCommand.deactivate();
		}

		if(m_vertSyncCommand.isActivated())
		{
			// Toggle the vertical synchronization
			WindowLocator().get().setVerticalSync(!Config::windowVar().vertical_sync);

			// Deactivate the key command so this piece of code is not triggered again if the key wasn't released
			m_vertSyncCommand.deactivate();
		}

		m_elapsedTime += p_deltaTime;
		if(m_elapsedTime > m_showFPSInterval)
		{
			m_elapsedTime -= m_showFPSInterval;

			std::string vsyncString = Config::windowVar().vertical_sync ? "VSYNC: ON" : "VSYNC: OFF";

			WindowLocator::get().setWindowTitle(Config::windowVar().name + " | " + vsyncString + " | FPS: " +
												Utilities::toString(roundf(ClockLocator::get().getFPS())));
		}
	}

	inline void setMouseCaptureKey(Scancode p_key)
	{
		m_mouseCaptureCommand.unbindAll();
		m_mouseCaptureCommand.bind(p_key);
	}
	inline void setMouseCaptureKey(std::string &p_string)
	{
		m_mouseCaptureCommand.unbindAll();
		m_mouseCaptureCommand.bind(p_string);
	}

	inline void setFullscreenKey(Scancode p_key)
	{
		m_fullscreenCommand.unbindAll();
		m_fullscreenCommand.bind(p_key);
	}
	inline void setFullscreenKey(std::string &p_string)
	{
		m_fullscreenCommand.unbindAll();
		m_fullscreenCommand.bind(p_string);
	}

	inline void setVerticalSyncKey(Scancode p_key)
	{
		m_vertSyncCommand.unbindAll();
		m_vertSyncCommand.bind(p_key);
	}
	inline void setVerticalSyncKey(std::string &p_string)
	{
		m_vertSyncCommand.unbindAll();
		m_vertSyncCommand.bind(p_string);
	}

	inline void setCloseWindowKey(Scancode p_key)
	{
		m_closeWindowCommand.unbindAll();
		m_closeWindowCommand.bind(p_key);
	}
	inline void setCloseWindowKey(std::string &p_string)
	{
		m_closeWindowCommand.unbindAll();
		m_closeWindowCommand.bind(p_string);
	}
	
protected:
	KeyCommand	m_fullscreenCommand, 
				m_mouseCaptureCommand,
				m_vertSyncCommand,
				m_closeWindowCommand;

	float	m_elapsedTime,
			m_showFPSInterval;
};