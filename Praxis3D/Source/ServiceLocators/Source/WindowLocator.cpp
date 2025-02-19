#include "Systems/GUISystem/Include/GUIHandler.hpp"
#include "ServiceLocators/Include/WindowLocator.hpp"

WindowLocator::WindowWrapperBase WindowLocator::m_nullWindowWrapper;
WindowLocator::WindowWrapperBase *WindowLocator::m_windowWrapper = &m_nullWindowWrapper;