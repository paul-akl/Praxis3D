#include "ServiceLocators/Include/GUIHandlerLocator.hpp"

GUIHandlerNull GUIHandlerLocator::m_nullGUIHandler;
GUIHandlerBase *GUIHandlerLocator::m_GUIHandler = &m_nullGUIHandler;