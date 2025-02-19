#pragma once

#include "Systems/GUISystem/Include/GUIHandler.hpp"

// Service locator provides global access to specific intanciated classes (i.e. services),
// but retains the ability to switch instances, and has fail-proof mechanism that returns 
// an empty (null) service if it hasn't been initialized. Uses dependency injection pattern.
class GUIHandlerLocator
{
	friend class EditorWindow;
	friend class Engine;
	friend class GUIPass;
	friend class GUIScene;
private:

	// Get the services
	inline static GUIHandlerBase &get() { return *m_GUIHandler; }

	// Initialize the service locator to use null services
	inline static ErrorCode init() { m_GUIHandler = &m_nullGUIHandler; return ErrorCode::Success; }

	// If the passed service is null, assign current service as a null service, otherwise assign the passed service as current
	inline static void provide(GUIHandlerBase *p_service) { m_GUIHandler = (p_service == nullptr) ? &m_nullGUIHandler : p_service; }


	static GUIHandlerBase *m_GUIHandler;
	static GUIHandlerNull m_nullGUIHandler;
};